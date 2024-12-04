#include "osr_example.h"
#include <lpp/location_information.h>
#include <lpp/lpp.h>
#include <sstream>
#include <stdexcept>

#include <format/ctrl/cid.hpp>
#include <format/ctrl/identity.hpp>
#include <format/ctrl/parser.hpp>
#include <format/nmea/message.hpp>
#include <format/nmea/parser.hpp>
#include <format/ubx/message.hpp>
#include <format/ubx/parser.hpp>
#include <io/input.hpp>
#include <io/output.hpp>
#include <scheduler/scheduler.hpp>
#include <streamline/system.hpp>

#include "location_information.h"

#ifdef INCLUDE_GENERATOR_RTCM
#include <generator/rtcm/generator.hpp>
using RtcmGenerator = std::unique_ptr<generator::rtcm::Generator>;
static RtcmGenerator                  gGenerator;
static generator::rtcm::MessageFilter gFilter;
#endif

static CellID              gCell;
static osr_example::Format gFormat;
static int                 gLrfRtcmId;
static Options             gOptions;
static bool                gPrintRtcm;

#include "ctrl.hpp"
#include "lpp.hpp"
#include "lpp2rtcm.hpp"
#include "nmea.hpp"
#include "ubx.hpp"

static streamline::System gStream;

static bool assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message*, void*);

static void initialize_inputs(scheduler::Scheduler&           scheduler,
                              std::vector<InputOption> const& inputs) {
    for (auto const& input : inputs) {
        format::nmea::Parser* nmea{};
        if ((input.format & INPUT_FORMAT_NMEA) != 0) {
            nmea = new format::nmea::Parser{};
        }

        format::ubx::Parser* ubx{};
        if ((input.format & INPUT_FORMAT_UBX) != 0) {
            ubx = new format::ubx::Parser{};
        }

        format::ctrl::Parser* ctrl{};
        if ((input.format & INPUT_FORMAT_CTRL) != 0) {
            ctrl = new format::ctrl::Parser{};
        }

        input.interface->schedule(scheduler);
        input.interface->callback = [nmea, ubx, ctrl](io::Input&, uint8_t* buffer, size_t count) {
            if (nmea) {
                // TODO(ewasjon): handle count > 2^16
                nmea->append(buffer, static_cast<uint16_t>(count));
                for (;;) {
                    auto message = nmea->try_parse();
                    if (!message) break;
                    gStream.push(std::move(message));
                }
            }

            if (ubx) {
                // TODO(ewasjon): handle count > 2^16
                ubx->append(buffer, static_cast<uint16_t>(count));
                for (;;) {
                    auto message = ubx->try_parse();
                    if (!message) break;
                    gStream.push(std::move(message));
                }
            }

            if (ctrl) {
                // TODO(ewasjon): handle count > 2^16
                ctrl->append(buffer, static_cast<uint16_t>(count));
                for (;;) {
                    auto message = ctrl->try_parse();
                    if (!message) break;
                    gStream.push(std::move(message));
                }
            }
        };
    }
}

static void initialize_outputs(OutputOptions const& outputs) {
    bool lpp_xer_output  = false;
    bool lpp_uper_output = false;
    bool nmea_output     = false;
    bool ubx_output      = false;
    bool ctrl_output     = false;
    for (auto& output : outputs.outputs) {
        if ((output.format & OUTPUT_FORMAT_LPP_XER) != 0) lpp_xer_output = true;
        if ((output.format & OUTPUT_FORMAT_LPP_UPER) != 0) lpp_uper_output = true;
        if ((output.format & OUTPUT_FORMAT_NMEA) != 0) nmea_output = true;
        if ((output.format & OUTPUT_FORMAT_UBX) != 0) ubx_output = true;
        if ((output.format & OUTPUT_FORMAT_CTRL) != 0) ctrl_output = true;
    }

    if (lpp_xer_output) gStream.add_inspector<LppXerOutput>(outputs);
    if (lpp_uper_output) gStream.add_inspector<LppUperOutput>(outputs);
    if (nmea_output) gStream.add_inspector<NmeaOutput>(outputs);
    if (ubx_output) gStream.add_inspector<UbxOutput>(outputs);
    if (ctrl_output) gStream.add_inspector<CtrlOutput>(outputs);
}

[[noreturn]] void execute(Options options, osr_example::Format format, int lrf_rtcm_id,
                          osr_example::MsmType msm_type, bool print_rtcm, bool gps, bool glonass,
                          bool galileo, bool beidou) {
    gOptions   = std::move(options);
    gFormat    = format;
    gLrfRtcmId = lrf_rtcm_id;
    gPrintRtcm = print_rtcm;

    loglet::set_level(gOptions.log_level);
    for (auto const& [module, level] : gOptions.module_levels) {
        loglet::set_module_level(module.c_str(), level);
    }

    auto& cell_options            = gOptions.cell_options;
    auto& location_server_options = gOptions.location_server_options;
    auto& identity_options        = gOptions.identity_options;
    auto& input_options           = gOptions.input_options;
    auto& output_options          = gOptions.output_options;
    // auto& ublox_options                = gOptions.ublox_options;
    // auto& nmea_options                 = gOptions.nmea_options;
    auto& location_information_options = gOptions.location_information_options;

    gCell.mcc   = cell_options.mcc;
    gCell.mnc   = cell_options.mnc;
    gCell.tac   = cell_options.tac;
    gCell.cell  = cell_options.cid;
    gCell.is_nr = cell_options.is_nr;

    printf("[settings]\n");
    printf("  location server:    \"%s:%d\" %s\n", location_server_options.host.c_str(),
           location_server_options.port, location_server_options.ssl ? "[ssl]" : "");
    printf("  identity:           ");
    if (identity_options.imsi)
        printf("imsi: %llu\n", *identity_options.imsi);
    else if (identity_options.msisdn)
        printf("msisdn: %llu\n", *identity_options.msisdn);
    else if (identity_options.ipv4)
        printf("ipv4: %s\n", identity_options.ipv4->c_str());
    else
        printf("none\n");
    printf("  cell information:   %s %ld:%ld:%ld:%llu (mcc:mnc:tac:id)\n",
           cell_options.is_nr ? "[nr]" : "[lte]", gCell.mcc, gCell.mnc, gCell.tac, gCell.cell);

    scheduler::Scheduler scheduler{};
    gStream = streamline::System{scheduler};

    if (input_options.print_ubx) {
        gStream.add_inspector<UbxPrint>();
    }

    if (input_options.print_nmea) {
        gStream.add_inspector<NmeaPrint>();
    }

    if (input_options.print_ctrl) {
        gStream.add_inspector<CtrlPrint>();
    }

    gStream.add_inspector<UbxLocation>(location_information_options.convert_confidence_95_to_39,
                                       location_information_options.override_horizontal_confidence,
                                       location_information_options.output_ellipse_68);
    gStream.add_consumer<NmeaLocation>(location_information_options.convert_confidence_95_to_39,
                                       location_information_options.override_horizontal_confidence,
                                       location_information_options.output_ellipse_68);

    gStream.add_inspector<LocationCollector>();
    gStream.add_inspector<MetricsCollector>();

    initialize_inputs(scheduler, input_options.inputs);
    initialize_outputs(output_options);

#ifdef INCLUDE_GENERATOR_RTCM
    if (gFormat == osr_example::Format::RTCM) {
        // Enable generation of message for GPS, GLONASS, Galileo, and Beidou.
        gFilter                 = generator::rtcm::MessageFilter{};
        gFilter.systems.gps     = gps;
        gFilter.systems.glonass = glonass;
        gFilter.systems.galileo = galileo;
        gFilter.systems.beidou  = beidou;

        printf("  gnss support:      ");
        if (gFilter.systems.gps) printf(" GPS");
        if (gFilter.systems.glonass) printf(" GLO");
        if (gFilter.systems.galileo) printf(" GAL");
        if (gFilter.systems.beidou) printf(" BDS");
        printf("\n");

        // Force MSM type if requested.
        switch (msm_type) {
        case osr_example::MsmType::MSM4: gFilter.msm.force_msm4 = true; break;
        case osr_example::MsmType::MSM5: gFilter.msm.force_msm5 = true; break;
        case osr_example::MsmType::MSM6: gFilter.msm.force_msm6 = true; break;
        case osr_example::MsmType::MSM7: gFilter.msm.force_msm7 = true; break;
        case osr_example::MsmType::ANY: break;
        }

        printf("  msm support:       ");
        if (gFilter.msm.force_msm4)
            printf(" MSM4");
        else if (gFilter.msm.force_msm5)
            printf(" MSM5");
        else if (gFilter.msm.force_msm6)
            printf(" MSM6");
        else if (gFilter.msm.force_msm7)
            printf(" MSM7");
        else {
            if (gFilter.msm.msm4) printf(" MSM4");
            if (gFilter.msm.msm5) printf(" MSM5");
            if (gFilter.msm.msm5) printf(" MSM6");
            if (gFilter.msm.msm7) printf(" MSM7");
        }
        printf("\n");

        // Create RTCM generator for converting LPP messages to RTCM messages.
        gGenerator = std::unique_ptr<generator::rtcm::Generator>(new generator::rtcm::Generator());
        gStream.add_inspector<Lpp2Rtcm>(gGenerator.get(), gFilter, gPrintRtcm, output_options);
    }
#endif

#ifdef INCLUDE_GENERATOR_RTCM
    if (gFormat == osr_example::Format::LRF_UPER) {
        gStream.add_inspector<LppRtcmFramedOutput>(output_options, lrf_rtcm_id);
    }
#endif

    LPP_Client::AD_Request request;
    LPP_Client             client{false /* enable experimental segmentation support */};
    bool                   client_initialized  = false;
    bool                   client_got_identity = false;

    if (!identity_options.use_supl_identity_fix) {
        client.use_incorrect_supl_identity();
    }

    auto ctrl_events = gStream.add_inspector<CtrlEvents>();
    if (ctrl_events) {
        ctrl_events->on_cell_id = [&](format::ctrl::CellId const& cell) {
            if (!client_initialized) return;
            CellID new_cell{};
            new_cell.mcc   = cell.mcc();
            new_cell.mnc   = cell.mnc();
            new_cell.tac   = cell.tac();
            new_cell.cell  = cell.cell();
            new_cell.is_nr = cell.is_nr();

            if (gCell != new_cell) {
                printf("[control] cell: %ld:%ld:%ld:%llu\n", new_cell.mcc, new_cell.mnc,
                       new_cell.tac, new_cell.cell);
                gCell = new_cell;
                client.update_assistance_data(request, gCell);
            } else {
                printf("[control] cell: %ld:%ld:%ld:%llu (unchanged)\n", new_cell.mcc, new_cell.mnc,
                       new_cell.tac, new_cell.cell);
            }
        };

        ctrl_events->on_identity_imsi = [&](format::ctrl::IdentityImsi const& identity) {
            printf("[control] identity: imsi: %" PRIu64 "\n", identity.imsi());
            if (client_got_identity) return;
            client.set_identity_imsi(identity.imsi());
            client_got_identity = true;
        };
    }

    if (identity_options.wait_for_identity) {
        printf("  waiting for identity\n");
        if (identity_options.imsi || identity_options.msisdn || identity_options.ipv4) {
            printf("  (imsi, msisdn, or ipv4 identity ignored)\n");
        }

        // TODO(ewasjon): Is there any way for us to know if we have a input for the control
        // messages? We don't want to stall here forever.
        scheduler.execute_while([&] {
            return !client_got_identity;
        });
    } else if (identity_options.imsi) {
        client.set_identity_imsi(*identity_options.imsi);
    } else if (identity_options.msisdn) {
        client.set_identity_msisdn(*identity_options.msisdn);
    } else if (identity_options.ipv4) {
        client.set_identity_ipv4(*identity_options.ipv4);
    } else {
        throw std::runtime_error("No identity provided");
    }

    printf("[location information]\n");
    if (location_information_options.fake_location_info) {
        printf("  source: simulated\n");
        client.provide_location_information_callback(&location_information_options,
                                                     provide_location_information_callback_fake);
    } else {
        printf("  source: streamline\n");
        client.provide_location_information_callback(
            nullptr, provide_location_information_callback_streamline);
    }

    if (location_information_options.force) {
        client.force_location_information();
        printf("  force: true\n");
    } else {
        printf("  force: false\n");
    }

    if (location_information_options.unlock_update_rate) {
        client.unlock_update_rate();
        printf("  unlock update rate: true\n");
    } else {
        printf("  unlock update rate: false\n");
    }

    if (!client.connect(location_server_options.host.c_str(), location_server_options.port,
                        location_server_options.ssl, gCell)) {
        throw std::runtime_error("Unable to connect to location server");
    }

    // Request OSR assistance data from location server for the 'cell' and register a callback
    // that will be called when we receive assistance data.
    request = client.request_assistance_data(gCell, nullptr, assistance_data_callback);
    if (request == AD_REQUEST_INVALID) {
        throw std::runtime_error("Unable to request assistance data");
    }

    client_initialized = true;

    for (;;) {
        scheduler.execute_timeout(std::chrono::milliseconds(100));

        // client.process() MUST be called at least once every second, otherwise
        // ProvideLocationInformation messages will not be send to the server.
        if (!client.process()) {
            throw std::runtime_error("Unable to process LPP client (probably disconnected)");
        }
    }
}

static bool assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message* message, void*) {
    gStream.push(LppMessage{message});
    return true;
}

namespace osr_example {

void OsrCommand::parse(args::Subparser& parser) {
    // NOTE: parse may be called multiple times
    delete mFormatArg;
    delete mLRFMessageIdArg;
    delete mMsmTypeArg;
    delete mPrintRTCMArg;

    mFormatArg = new args::ValueFlag<std::string>(parser, "format", "Format", {'f', "format"},
                                                  args::Options::Single);
#ifdef INCLUDE_GENERATOR_RTCM
    mFormatArg->HelpDefault("rtcm");
#else
    mFormatArg->HelpDefault("xer");
#endif
    mFormatArg->HelpChoices({
        "xer",
        "asn1-uper",
#ifdef INCLUDE_GENERATOR_RTCM
        "rtcm",
        "lrf-uper",
#endif
        "none",
    });

    mLRFMessageIdArg =
        new args::ValueFlag<int>(parser, "lrf-message-id", "RTCM message ID for LRF-UPER format",
                                 {"lrf-message-id"}, args::Options::Single);
    mLRFMessageIdArg->HelpDefault("355");

    mMsmTypeArg = new args::ValueFlag<std::string>(parser, "msm_type", "RTCM MSM type",
                                                   {'y', "msm_type"}, args::Options::Single);
    mMsmTypeArg->HelpDefault("any");
    mMsmTypeArg->HelpChoices({"any", "4", "5", "6", "7"});

    // the default value is true, thus this is a negated flag
    mPrintRTCMArg = new args::Flag(parser, "print_rtcm", "Do not print RTCM messages info",
                                   {"rtcm-print"}, args::Options::Single);

    mNoGPS     = new args::Flag(parser, "no-gps", "Skip generating GPS RTCM messages", {"no-gps"});
    mNoGLONASS = new args::Flag(parser, "no-glonass", "Skip generating GLONASS RTCM messages",
                                {"no-glonass"});
    mNoGalileo = new args::Flag(parser, "no-galileo", "Skip generating Galileo RTCM messages",
                                {"no-galileo"});
    mNoBeiDou =
        new args::Flag(parser, "no-beidou", "Skip generating BeiDou RTCM messages", {"no-beidou"});
}

void OsrCommand::execute(Options options) {
#ifdef INCLUDE_GENERATOR_RTCM
    auto format = Format::RTCM;
#else
    auto format = Format::XER;
#endif

    if (*mFormatArg) {
        if (mFormatArg->Get() == "xer") {
            format = Format::XER;
        } else if (mFormatArg->Get() == "asn1-uper") {
            format = Format::ASN1_UPER;
        } else if (mFormatArg->Get() == "none") {
            format = Format::NONE;
        }
#ifdef INCLUDE_GENERATOR_RTCM
        else if (mFormatArg->Get() == "rtcm") {
            format = Format::RTCM;
        } else if (mFormatArg->Get() == "lrf-uper") {
            format = Format::LRF_UPER;
        }
#endif
        else {
            throw args::ValidationError("Invalid format");
        }
    }

    auto lrf_rtcm_id = 355;
    if (*mLRFMessageIdArg) {
        lrf_rtcm_id = mLRFMessageIdArg->Get();
    }

    auto msm_type = MsmType::ANY;
    if (*mMsmTypeArg) {
        if (mMsmTypeArg->Get() == "any") {
            msm_type = MsmType::ANY;
        } else if (mMsmTypeArg->Get() == "4") {
            msm_type = MsmType::MSM4;
        } else if (mMsmTypeArg->Get() == "5") {
            msm_type = MsmType::MSM5;
        } else if (mMsmTypeArg->Get() == "6") {
            msm_type = MsmType::MSM6;
        } else if (mMsmTypeArg->Get() == "7") {
            msm_type = MsmType::MSM7;
        } else {
            throw args::ValidationError("Invalid MSM type");
        }
    }

    auto print_rtcm = true;
    if (*mPrintRTCMArg) {
        print_rtcm = false;
    }

    auto gps     = true;
    auto glonass = true;
    auto galileo = true;
    auto beidou  = true;
    if (*mNoGPS) {
        gps = false;
    }
    if (*mNoGLONASS) {
        glonass = false;
    }
    if (*mNoGalileo) {
        galileo = false;
    }
    if (*mNoBeiDou) {
        beidou = false;
    }

    ::execute(std::move(options), format, lrf_rtcm_id, msm_type, print_rtcm, gps, glonass, galileo,
              beidou);
}

}  // namespace osr_example
