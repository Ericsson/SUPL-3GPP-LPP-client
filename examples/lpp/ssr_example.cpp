#include "ssr_example.h"
#include <iostream>
#include <lpp/location_information.h>
#include <lpp/lpp.h>
#include <receiver/nmea/threaded_receiver.hpp>
#include <receiver/ublox/threaded_receiver.hpp>
#include <sstream>
#include <stdexcept>

#include "control.hpp"
#include "location_information.h"

#ifdef INCLUDE_GENERATOR_SPARTN_OLD
#include <generator/spartn/generator.h>
#include <generator/spartn/transmitter.h>
#endif

#ifdef INCLUDE_GENERATOR_SPARTN
#include <generator/spartn2/generator.hpp>
#endif

#ifdef INCLUDE_GENERATOR_RTCM
#include <generator/rtcm/generator.hpp>
#endif

using UReceiver = receiver::ublox::ThreadedReceiver;
using NReceiver = receiver::nmea::ThreadedReceiver;

static CellID              gCell;
static ssr_example::Format gFormat;
static int                 gUraOverride;
static int                 gUraDefault;
static bool                gUBloxClockCorrection;
static bool                gForceIodeContinuity;
static bool                gAverageZenithDelay;
static bool                gEnableIodeShift;
static int                 gSf055Override;
static int                 gSf055Default;
static int                 gSf042Override;
static int                 gSf042Default;
static bool                gIncreasingSiou;
static bool                gFilterByOcb;
static bool                gIgnoreL2L;
static bool                gPrintRtcm;
static Options             gOptions;
static ControlParser       gControlParser;

#ifdef INCLUDE_GENERATOR_SPARTN_OLD
static SPARTN_Generator gSpartnGeneratorOld;
#endif

#ifdef INCLUDE_GENERATOR_SPARTN
static generator::spartn::Generator gSpartnGeneratorNew;
#endif

static std::unique_ptr<UReceiver> gUbloxReceiver;
static std::unique_ptr<NReceiver> gNmeaReceiver;

static void assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message*, void*);

[[noreturn]] void execute(Options options, ssr_example::Format format, int ura_override,
                          int ura_default, bool ublox_clock_correction, bool force_continuity,
                          bool average_zenith_delay, bool enable_iode_shift, int sf055_override,
                          int sf055_default, int sf042_override, int sf042_default,
                          bool increasing_siou, bool filter_by_ocb, bool ignore_l2l,
                          bool print_rtcm) {
    gOptions              = std::move(options);
    gFormat               = format;
    gUraOverride          = ura_override;
    gUraDefault           = ura_default;
    gUBloxClockCorrection = ublox_clock_correction;
    gForceIodeContinuity  = force_continuity;
    gAverageZenithDelay   = average_zenith_delay;
    gEnableIodeShift      = enable_iode_shift;
    gSf055Override        = sf055_override;
    gSf055Default         = sf055_default;
    gSf042Override        = sf042_override;
    gSf042Default         = sf042_default;
    gIncreasingSiou       = increasing_siou;
    gFilterByOcb          = filter_by_ocb;
    gIgnoreL2L            = ignore_l2l;
    gPrintRtcm            = print_rtcm;

    auto& cell_options                 = gOptions.cell_options;
    auto& location_server_options      = gOptions.location_server_options;
    auto& identity_options             = gOptions.identity_options;
    auto& output_options               = gOptions.output_options;
    auto& ublox_options                = gOptions.ublox_options;
    auto& nmea_options                 = gOptions.nmea_options;
    auto& location_information_options = gOptions.location_information_options;
    auto& control_options              = gOptions.control_options;

    gConvertConfidence95To39      = location_information_options.convert_confidence_95_to_39;
    gOverrideHorizontalConfidence = location_information_options.override_horizontal_confidence;

    gCell.mcc   = cell_options.mcc;
    gCell.mnc   = cell_options.mnc;
    gCell.tac   = cell_options.tac;
    gCell.cell  = cell_options.cid;
    gCell.is_nr = cell_options.is_nr;

    printf("[settings]\n");
    printf("  location server:    \"%s:%d\" %s\n", location_server_options.host.c_str(),
           location_server_options.port, location_server_options.ssl ? "[ssl]" : "");
    printf("  identity:           ");
    if (identity_options.wait_for_identity)
        printf("identity from control\n");
    else if (identity_options.imsi)
        printf("imsi: %llu\n", *identity_options.imsi);
    else if (identity_options.msisdn)
        printf("msisdn: %llu\n", *identity_options.msisdn);
    else if (identity_options.ipv4)
        printf("ipv4: %s\n", identity_options.ipv4->c_str());
    else
        printf("none\n");
    printf("  cell information:   %s %ld:%ld:%ld:%llu (mcc:mnc:tac:id)\n",
           cell_options.is_nr ? "[nr]" : "[lte]", gCell.mcc, gCell.mnc, gCell.tac, gCell.cell);

    for (auto& interface : output_options.interfaces) {
        interface->open();
        interface->print_info();
    }

    if (ublox_options.interface) {
        printf("[ublox]\n");
        ublox_options.interface->open();
        ublox_options.interface->print_info();

        gUbloxReceiver = std::unique_ptr<UReceiver>(new UReceiver(
            ublox_options.port, std::move(ublox_options.interface), ublox_options.print_messages));
        gUbloxReceiver->start();
    }

    if (nmea_options.interface) {
        printf("[nmea]\n");
        nmea_options.interface->open();
        nmea_options.interface->print_info();

        if (!nmea_options.export_interfaces.empty()) {
            printf("[nmea-export]\n");
            for (auto& interface : nmea_options.export_interfaces) {
                interface->open();
                interface->print_info();
            }
        }

        gNmeaReceiver = std::unique_ptr<NReceiver>(
            new NReceiver(std::move(nmea_options.interface), nmea_options.print_messages,
                          std::move(nmea_options.export_interfaces)));
        gNmeaReceiver->start();
    }

#ifdef INCLUDE_GENERATOR_SPARTN
    gSpartnGeneratorNew.set_ura_override(gUraOverride);
    gSpartnGeneratorNew.set_ura_default(gUraDefault);
    gSpartnGeneratorNew.set_ublox_clock_correction(gUBloxClockCorrection);
    if (gForceIodeContinuity) {
        gSpartnGeneratorNew.set_continuity_indicator(320.0);
    }
    if (gAverageZenithDelay) {
        gSpartnGeneratorNew.set_compute_average_zenith_delay(true);
    }
    if (gEnableIodeShift) {
        gSpartnGeneratorNew.set_iode_shift(true);
    } else {
        gSpartnGeneratorNew.set_iode_shift(false);
    }
    if (gSf055Override >= 0) gSpartnGeneratorNew.set_sf055_override(gSf055Override);
    if (gSf055Default >= 0) gSpartnGeneratorNew.set_sf055_default(gSf055Default);
    if (gSf042Override >= 0) gSpartnGeneratorNew.set_sf042_override(gSf042Override);
    if (gSf042Default >= 0) gSpartnGeneratorNew.set_sf042_default(gSf042Default);

    if (gIncreasingSiou) gSpartnGeneratorNew.set_increasing_siou(true);
    if (gFilterByOcb) gSpartnGeneratorNew.set_filter_by_ocb(true);
    if (gIgnoreL2L) gSpartnGeneratorNew.set_ignore_l2l(true);

#endif

    LPP_Client::AD_Request request;
    LPP_Client             client{false /* experimental segmentation support */};
    bool                   client_initialized  = false;
    bool                   client_got_identity = false;

    if (!identity_options.use_supl_identity_fix) {
        client.use_incorrect_supl_identity();
    }

    if (control_options.interface) {
        printf("[control]\n");
        control_options.interface->open();
        control_options.interface->print_info();

        gControlParser.on_cid = [&](CellID cell) {
            if (!client_initialized) return;
            if (gCell != cell) {
                printf("[control] cell: %ld:%ld:%ld:%llu\n", cell.mcc, cell.mnc, cell.tac,
                       cell.cell);
                gCell = cell;
                client.update_assistance_data(request, gCell);
            } else {
                printf("[control] cell: %ld:%ld:%ld:%llu (unchanged)\n", cell.mcc, cell.mnc,
                       cell.tac, cell.cell);
            }
        };

        gControlParser.on_identity_imsi = [&](unsigned long long imsi) {
            printf("[control] identity: imsi: %llu\n", imsi);
            if (client_got_identity) return;
            client.set_identity_imsi(imsi);
            client_got_identity = true;
        };
    }

    if (identity_options.wait_for_identity) {
        if (!control_options.interface) {
            throw std::runtime_error("No control interface provided");
        }

        printf("  waiting for identity\n");
        if (identity_options.imsi || identity_options.msisdn || identity_options.ipv4) {
            printf("  (imsi, msisdn, or ipv4 identity ignored)\n");
        }
        while (!client_got_identity) {
            struct timespec timeout;
            timeout.tv_sec  = 0;
            timeout.tv_nsec = 1000000 * 100;  // 100 ms
            nanosleep(&timeout, nullptr);

            gControlParser.parse(control_options.interface);
        }
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
    if (gUbloxReceiver.get()) {
        printf("  source: ublox\n");
        client.provide_location_information_callback(gUbloxReceiver.get(),
                                                     provide_location_information_callback_ublox);
    } else if (gNmeaReceiver.get()) {
        printf("  source: nmea\n");
        client.provide_location_information_callback(gNmeaReceiver.get(),
                                                     provide_location_information_callback_nmea);
    } else if (location_information_options.enabled) {
        printf("  source: simulated\n");
        client.provide_location_information_callback(&location_information_options,
                                                     provide_location_information_callback_fake);
    } else {
        printf("  source: none\n");
        client.provide_location_information_callback(nullptr,
                                                     provide_location_information_callback);
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

    request = client.request_assistance_data_ssr(gCell, nullptr, assistance_data_callback);
    if (request == AD_REQUEST_INVALID) {
        throw std::runtime_error("Unable to request assistance data");
    }

    client_initialized = true;

    for (;;) {
        struct timespec timeout;
        timeout.tv_sec  = 0;
        timeout.tv_nsec = 1000000 * 100;  // 100 ms
        nanosleep(&timeout, nullptr);

        // client.process() MUST be called at least once every second, otherwise
        // ProvideLocationInformation messages will not be send to the server.
        if (!client.process()) {
            throw std::runtime_error("Unable to process LPP client (probably disconnected)");
        }

        if (control_options.interface) {
            gControlParser.parse(control_options.interface);
        }
    }
}

static void assistance_data_callback(LPP_Client* client, LPP_Transaction*, LPP_Message* message,
                                     void*) {
    if (gFormat == ssr_example::Format::XER) {
        std::stringstream buffer;
        xer_encode(
            &asn_DEF_LPP_Message, message, XER_F_BASIC,
            [](void const* text_buffer, size_t text_size, void* app_key) -> int {
                auto string_stream = static_cast<std::ostream*>(app_key);
                string_stream->write(static_cast<char const*>(text_buffer),
                                     static_cast<std::streamsize>(text_size));
                return 0;
            },
            &buffer);
        auto xer_message = buffer.str();
        for (auto& interface : gOptions.output_options.interfaces) {
            interface->write(xer_message.c_str(), xer_message.size());
        }
    } else if (gFormat == ssr_example::Format::ASN1_UPER) {
        auto octet = client->encode(message);
        if (octet) {
            for (auto& interface : gOptions.output_options.interfaces) {
                interface->write(octet->buf, octet->size);
            }

            ASN_STRUCT_FREE(asn_DEF_OCTET_STRING, octet);
        }
    }
#ifdef INCLUDE_GENERATOR_SPARTN_OLD
    else if (gFormat == ssr_example::Format::SPARTN_OLD) {
        auto messages = gSpartnGeneratorOld.generate(message, gUraOverride, gUBloxClockCorrection,
                                                     gForceIodeContinuity);
        for (auto& msg : messages) {
            auto bytes = SPARTN_Transmitter::build(msg);
            for (auto& interface : gOptions.output_options.interfaces) {
                interface->write(bytes.data(), bytes.size());
            }

            if (gUbloxReceiver) {
                auto interface = gUbloxReceiver->interface();
                if (interface) {
                    interface->write(bytes.data(), bytes.size());
                }
            } else if (gNmeaReceiver) {
                auto interface = gNmeaReceiver->interface();
                if (interface) {
                    interface->write(bytes.data(), bytes.size());
                }
            }
        }
    }
#endif
#ifdef INCLUDE_GENERATOR_SPARTN
    else if (gFormat == ssr_example::Format::SPARTN_NEW) {
        auto messages = gSpartnGeneratorNew.generate(message);
        for (auto& msg : messages) {
            auto data = msg.build();
            if (data.size() == 0) {
                printf("Size of SPARTN payload is above the 1024 byte limit\n");
                continue;
            }

            for (auto& interface : gOptions.output_options.interfaces) {
                interface->write(data.data(), data.size());
            }

            if (gUbloxReceiver) {
                auto interface = gUbloxReceiver->interface();
                if (interface) {
                    interface->write(data.data(), data.size());
                }
            } else if (gNmeaReceiver) {
                auto interface = gNmeaReceiver->interface();
                if (interface) {
                    interface->write(data.data(), data.size());
                }
            }
        }
    }
#endif
#ifdef INCLUDE_GENERATOR_RTCM
    else if (gFormat == ssr_example::Format::LRF_UPER) {
        auto octet = client->encode(message);
        if (octet) {
            auto submessages =
                generator::rtcm::Generator::generate_framing(octet->buf, octet->size);

            if (gPrintRtcm) {
                size_t length = 0;
                for (auto& submessage : submessages) {
                    length += submessage.data().size();
                }

                printf("LRF: %4zu bytes | ", length);
                for (auto& submessage : submessages) {
                    printf("%4i ", submessage.id());
                }
                printf("\n");
            }

            for (auto& submessage : submessages) {
                auto buffer = submessage.data().data();
                auto size   = submessage.data().size();
                for (auto& interface : gOptions.output_options.interfaces) {
                    interface->write(buffer, size);
                }
            }

            if (gUbloxReceiver) {
                auto interface = gUbloxReceiver->interface();
                if (interface) {
                    for (auto& submessage : submessages) {
                        auto buffer = submessage.data().data();
                        auto size   = submessage.data().size();
                        interface->write(buffer, size);
                    }
                }
            } else if (gNmeaReceiver) {
                auto interface = gNmeaReceiver->interface();
                if (interface) {
                    for (auto& submessage : submessages) {
                        auto buffer = submessage.data().data();
                        auto size   = submessage.data().size();
                        interface->write(buffer, size);
                    }
                }
            }

            ASN_STRUCT_FREE(asn_DEF_OCTET_STRING, octet);
        }
    }
#endif
    else {
        throw std::runtime_error("Unsupported format");
    }
}

namespace ssr_example {

void SsrCommand::parse(args::Subparser& parser) {
    // NOTE: parse may be called multiple times
    delete mFormatArg;
    delete mUraOverrideArg;
    delete mUraDefaultArg;
    delete mUbloxClockCorrectionArg;
    delete mForceContinuityArg;
    delete mAverageZenithDelayArg;
    delete mEnableIodeShift;
    delete mSf055Override;
    delete mSf055Default;
    delete mSf042Override;
    delete mSf042Default;
    delete mIncreasingSiou;
    delete mFilterByOcb;
    delete mIgnoreL2L;
    delete mPrintRTCMArg;

    mFormatArg = new args::ValueFlag<std::string>(parser, "format", "Format of the output",
                                                  {"format"}, args::Options::Single);
    mFormatArg->HelpDefault("xer");
    mFormatArg->HelpChoices({
        "xer",
        "asn1-uper",
#ifdef INCLUDE_GENERATOR_SPARTN
        "spartn",
#endif
#ifdef INCLUDE_GENERATOR_SPARTN_OLD
        "spartn-old",
#endif
#ifdef INCLUDE_GENERATOR_RTCM
        "lrf-uper",
#endif
    });

    mUraOverrideArg = new args::ValueFlag<int>(
        parser, "ura-override",
        "Override the URA (SF024) value, value will be clamped between 0-7. "
        "Where 0 indicates that the value is unknown.",
        {"ura-override"}, args::Options::Single);
    mUraDefaultArg = new args::ValueFlag<int>(
        parser, "ura-default",
        "Set the default URA (SF024) value, value will be clamped between 0-7. "
        "Where 0 indicates that the value is unknown.",
        {"ura-default"}, args::Options::Single);

    mUbloxClockCorrectionArg =
        new args::Flag(parser, "ublox-clock-correction", "Change the sign of the clock correction",
                       {"ublox-clock-correction"});
    mForceContinuityArg =
        new args::Flag(parser, "force-continuity", "Force SF022 (IODE Continuity) to be 320 secs",
                       {"force-continuity"});

    mAverageZenithDelayArg =
        new args::Flag(parser, "average-zenith-delay",
                       "Compute the average zenith delay and differential for residuals",
                       {"average-zenith-delay"});

    mEnableIodeShift = new args::Flag(
        parser, "iode-shift", "Enable the IODE shift to fix data stream issues", {"iode-shift"});

    mSf055Override =
        new args::ValueFlag<int>(parser, "sf055-override",
                                 "Override the SF055 value, value will be clamped between 0-15. "
                                 "Where 0 indicates that the value is invalid.",
                                 {"sf055-override"}, args::Options::Single);
    mSf055Default =
        new args::ValueFlag<int>(parser, "sf055-default",
                                 "Set the default SF055 value, value will be clamped between 0-15. "
                                 "Where 0 indicates that the value is invalid.",
                                 {"sf055-default"}, args::Options::Single);

    mSf042Override = new args::ValueFlag<int>(
        parser, "sf042-override", "Override the SF042 value, value will be clamped between 0-7.",
        {"sf042-override"}, args::Options::Single);
    mSf042Default = new args::ValueFlag<int>(
        parser, "sf042-default", "Set the default SF042 value, value will be clamped between 0-7.",
        {"sf042-default"}, args::Options::Single);

    mIncreasingSiou =
        new args::Flag(parser, "increasing-siou", "Enable the increasing SIoU feature for SPARTN",
                       {"increasing-siou"});
    mFilterByOcb = new args::Flag(
        parser, "filter-by-ocb",
        "Only include ionospheric residual satellites that also have OCB corrections",
        {"filter-by-ocb"});
    mIgnoreL2L = new args::Flag(parser, "ignore-l2l", "Ignore L2L biases", {"ignore-l2l"});

    mPrintRTCMArg =
        new args::Flag(parser, "print_rtcm", "Print RTCM messages info (only used for LRF-UPER)",
                       {"rtcm-print"}, args::Options::Single);
}

void SsrCommand::execute(Options options) {
    auto format = ssr_example::Format::XER;
    if (*mFormatArg) {
        if (mFormatArg->Get() == "xer") {
            format = ssr_example::Format::XER;
        } else if (mFormatArg->Get() == "asn1-uper") {
            format = ssr_example::Format::ASN1_UPER;
        }
#ifdef INCLUDE_GENERATOR_SPARTN
        else if (mFormatArg->Get() == "spartn") {
            format = ssr_example::Format::SPARTN_NEW;
        }
#endif
#ifdef INCLUDE_GENERATOR_SPARTN_OLD
        else if (mFormatArg->Get() == "spartn-old") {
            format = ssr_example::Format::SPARTN_OLD;
        }
#endif
#ifdef INCLUDE_GENERATOR_RTCM
        else if (mFormatArg->Get() == "lrf-uper") {
            format = ssr_example::Format::LRF_UPER;
        }
#endif
        else {
            throw args::ValidationError("Invalid format");
        }
    }

    auto ura_override = -1;
    if (*mUraOverrideArg) {
        ura_override = mUraOverrideArg->Get();
    }

    auto ura_default = -1;
    if (*mUraDefaultArg) {
        ura_default = mUraDefaultArg->Get();
    }

    auto ublox_clock_correction = false;
    if (*mUbloxClockCorrectionArg) {
        ublox_clock_correction = mUbloxClockCorrectionArg->Get();
    }

    auto force_continuity = false;
    if (*mForceContinuityArg) {
        force_continuity = mForceContinuityArg->Get();
    }

    auto average_zenith_delay = false;
    if (*mAverageZenithDelayArg) {
        average_zenith_delay = mAverageZenithDelayArg->Get();
    }

    auto iode_shift = false;
    if (*mEnableIodeShift) {
        iode_shift = mEnableIodeShift->Get();
    }

    auto sf055_override = -1;
    if (*mSf055Override) {
        sf055_override = mSf055Override->Get();
        if (sf055_override < 0) sf055_override = 0;
        if (sf055_override > 15) sf055_override = 15;
    }

    auto sf055_default = -1;
    if (*mSf055Default) {
        sf055_default = mSf055Default->Get();
        if (sf055_default < 0) sf055_default = 0;
        if (sf055_default > 15) sf055_default = 15;
    }

    auto sf042_override = -1;
    if (*mSf042Override) {
        sf042_override = mSf042Override->Get();
        if (sf042_override < 0) sf042_override = 0;
        if (sf042_override > 7) sf042_override = 7;
    }

    auto sf042_default = -1;
    if (*mSf042Default) {
        sf042_default = mSf042Default->Get();
        if (sf042_default < 0) sf042_default = 0;
        if (sf042_default > 7) sf042_default = 7;
    }

    auto increasing_siou = false;
    if (*mIncreasingSiou) {
        increasing_siou = mIncreasingSiou->Get();
    }

    auto filter_by_ocb = false;
    if (*mFilterByOcb) {
        filter_by_ocb = mFilterByOcb->Get();
    }

    auto ignore_l2l = false;
    if (*mIgnoreL2L) {
        ignore_l2l = mIgnoreL2L->Get();
    }

    auto print_rtcm = false;
    if (*mPrintRTCMArg) {
        print_rtcm = true;
    }

    ::execute(std::move(options), format, ura_override, ura_default, ublox_clock_correction,
              force_continuity, average_zenith_delay, iode_shift, sf055_override, sf055_default,
              sf042_override, sf042_default, increasing_siou, filter_by_ocb, ignore_l2l,
              print_rtcm);
}

}  // namespace ssr_example
