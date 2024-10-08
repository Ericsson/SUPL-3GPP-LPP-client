#include "ssr_example.h"
#include <format/ctrl/cid.hpp>
#include <format/ctrl/identity.hpp>
#include <format/ctrl/parser.hpp>
#include <format/nmea/message.hpp>
#include <format/nmea/parser.hpp>
#include <format/ubx/message.hpp>
#include <format/ubx/parser.hpp>
#include <format/lpp/uper_parser.hpp>
#include <iostream>
#include <lpp/internal_lpp.h>
#include <lpp/location_information.h>
#include <lpp/lpp.h>
#include <sstream>
#include <stdexcept>

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

#include <io/input.hpp>
#include <io/output.hpp>
#include <scheduler/scheduler.hpp>
#include <streamline/system.hpp>

struct SsrGlobals {
    Options                       options;
    CellID                        cell;
    ssr_example::Format           format;
    int                           lrf_rtcm_id;
    int                           ura_override;
    int                           ura_default;
    bool                          ublox_clock_correction;
    bool                          force_continuity;
    bool                          average_zenith_delay;
    bool                          iode_shift;
    int                           sf055_override;
    int                           sf055_default;
    int                           sf042_override;
    int                           sf042_default;
    bool                          increasing_siou;
    bool                          filter_by_residuals;
    bool                          filter_by_ocb;
    bool                          ignore_l2l;
    bool                          print_rtcm;
    bool                          hydrostatic_in_zenith;
    generator::spartn::StecMethod stec_method;
    bool                          stec_transform;
    bool                          stec_invalid_to_zero;
    bool                          sign_flip_c00;
    bool                          sign_flip_c01;
    bool                          sign_flip_c10;
    bool                          sign_flip_c11;
    bool                          sign_flip_stec_residuals;
    bool                          code_bias_translate;
    bool                          code_bias_correction_shift;
    bool                          phase_bias_translate;
    bool                          phase_bias_correction_shift;
    bool                          generate_gps;
    bool                          generate_glonass;
    bool                          generate_galileo;
    bool                          generate_beidou;
    bool                          generate_gad;
    bool                          generate_ocb;
    bool                          generate_hpac;
    bool                          flip_grid_bitmask;
    bool                          flip_orbit_correction;
};

#include "ctrl.hpp"
#include "lpp.hpp"
#include "lpp2spartn.hpp"
#include "nmea.hpp"
#include "ubx.hpp"

#ifdef INCLUDE_GENERATOR_TOKORO
#include "tokoro.hpp"
#endif

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "ex/ssr"

static streamline::System gStream;
static SsrGlobals         gGlobals;

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

        format::lpp::UperParser* lpp_uper{};
        if ((input.format & INPUT_FORMAT_LPP) != 0) {
            lpp_uper = new format::lpp::UperParser{};
        }

        DEBUGF("input %p: %s%s%s%s", input.interface.get(),
               (input.format & INPUT_FORMAT_UBX) ? "UBX " : "",
               (input.format & INPUT_FORMAT_NMEA) ? "NMEA " : "",
               (input.format & INPUT_FORMAT_CTRL) ? "CTRL " : "",
               (input.format & INPUT_FORMAT_LPP) ? "LPP " : "");

        input.interface->schedule(scheduler);
        input.interface->callback = [nmea, ubx, ctrl, lpp_uper](io::Input&, uint8_t* buffer,
                                                                size_t count) {
            if (nmea) {
                nmea->append(buffer, count);
                for (;;) {
                    auto message = nmea->try_parse();
                    if (!message) break;
                    gStream.push(std::move(message));
                }
            }

            if (ubx) {
                ubx->append(buffer, count);
                for (;;) {
                    auto message = ubx->try_parse();
                    if (!message) break;
                    gStream.push(std::move(message));
                }
            }

            if (ctrl) {
                ctrl->append(buffer, count);
                for (;;) {
                    auto message = ctrl->try_parse();
                    if (!message) break;
                    gStream.push(std::move(message));
                }
            }

            if (lpp_uper) {
                lpp_uper->append(buffer, count);
                for (;;) {
                    auto message = lpp_uper->try_parse();
                    if (!message) break;
                    auto lpp_message = LppMessage{message};
                    gStream.push(std::move(lpp_message));
                }
            }
        };
    }
}

static void initialize_outputs(OutputOptions const& outputs) {
    VSCOPE_FUNCTION();

    bool lpp_xer_output  = false;
    bool lpp_uper_output = false;
    bool nmea_output     = false;
    bool ubx_output      = false;
    bool ctrl_output     = false;
    for (auto& output : outputs.outputs) {
        DEBUGF("%p: %s%s%s%s%s", output.interface.get(),
               (output.format & OUTPUT_FORMAT_UBX) ? "UBX " : "",
               (output.format & OUTPUT_FORMAT_NMEA) ? "NMEA " : "",
               (output.format & OUTPUT_FORMAT_CTRL) ? "CTRL " : "",
               (output.format & OUTPUT_FORMAT_LPP_XER) ? "LPP_XER " : "",
               (output.format & OUTPUT_FORMAT_LPP_UPER) ? "LPP_UPER " : "");

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

[[noreturn]] void execute() {
    auto& cell_options                 = gGlobals.options.cell_options;
    auto& location_server_options      = gGlobals.options.location_server_options;
    auto& identity_options             = gGlobals.options.identity_options;
    auto& output_options               = gGlobals.options.output_options;
    auto& input_options                = gGlobals.options.input_options;
    auto& location_information_options = gGlobals.options.location_information_options;

    loglet::set_level(gGlobals.options.log_level);
    for (auto const& [module, level] : gGlobals.options.module_levels) {
        loglet::set_module_level(module.c_str(), level);
    }

    INFOF("[settings]");
    INFOF("  location server:    \"%s:%d\" %s", location_server_options.host.c_str(),
          location_server_options.port, location_server_options.ssl ? "[ssl]" : "");
    if (identity_options.wait_for_identity)
        INFOF("  identity:           control");
    else if (identity_options.imsi)
        INFOF("  identity:           imsi: %llu", *identity_options.imsi);
    else if (identity_options.msisdn)
        INFOF("  identity:           msisdn: %llu", *identity_options.msisdn);
    else if (identity_options.ipv4)
        INFOF("  identity:           ipv4: %s", identity_options.ipv4->c_str());
    else
        INFOF("  identity:           none");
    INFOF("  cell information:   %s %ld:%ld:%ld:%llu (mcc:mnc:tac:id)",
          cell_options.is_nr ? "[nr]" : "[lte]", gGlobals.cell.mcc, gGlobals.cell.mnc,
          gGlobals.cell.tac, gGlobals.cell.cell);

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

    if (!location_information_options.disable_ubx_location) {
        INFOF("UBX location enabled");
        gStream.add_inspector<UbxLocation>(
            location_information_options.convert_confidence_95_to_39,
            location_information_options.override_horizontal_confidence,
            location_information_options.output_ellipse_68);
    }

    if (!location_information_options.disable_nmea_location) {
        INFOF("NMEA location enabled");
        gStream.add_consumer<NmeaLocation>(
            location_information_options.convert_confidence_95_to_39,
            location_information_options.override_horizontal_confidence,
            location_information_options.output_ellipse_68);
    }

    gStream.add_inspector<LocationCollector>();
    gStream.add_inspector<MetricsCollector>();

    initialize_inputs(scheduler, input_options.inputs);
    initialize_outputs(output_options);

#ifdef INCLUDE_GENERATOR_SPARTN
    if (gGlobals.format == ssr_example::Format::SPARTN_NEW) {
        auto  inspector = gStream.add_inspector<Lpp2Spartn>(output_options);
        auto& generator = inspector->generator();

        generator.set_ura_override(gGlobals.ura_override);
        generator.set_ura_default(gGlobals.ura_default);
        generator.set_ublox_clock_correction(gGlobals.ublox_clock_correction);
        if (gGlobals.force_continuity) {
            generator.set_continuity_indicator(320.0);
        }
        generator.set_compute_average_zenith_delay(gGlobals.average_zenith_delay);
        generator.set_iode_shift(gGlobals.iode_shift);

        if (gGlobals.sf055_override >= 0) generator.set_sf055_override(gGlobals.sf055_override);
        if (gGlobals.sf055_default >= 0) generator.set_sf055_default(gGlobals.sf055_default);
        if (gGlobals.sf042_override >= 0) generator.set_sf042_override(gGlobals.sf042_override);
        if (gGlobals.sf042_default >= 0) generator.set_sf042_default(gGlobals.sf042_default);

        generator.set_increasing_siou(gGlobals.increasing_siou);
        generator.set_filter_by_residuals(gGlobals.filter_by_residuals);
        generator.set_filter_by_ocb(gGlobals.filter_by_ocb);
        generator.set_ignore_l2l(gGlobals.ignore_l2l);
        generator.set_stec_invalid_to_zero(gGlobals.stec_invalid_to_zero);

        generator.set_code_bias_translate(gGlobals.code_bias_translate);
        generator.set_code_bias_correction_shift(gGlobals.code_bias_correction_shift);
        generator.set_phase_bias_translate(gGlobals.phase_bias_translate);
        generator.set_phase_bias_correction_shift(gGlobals.phase_bias_correction_shift);

        generator.set_hydrostatic_in_zenith(gGlobals.hydrostatic_in_zenith);
        generator.set_stec_method(gGlobals.stec_method);
        generator.set_stec_transform(gGlobals.stec_transform);
        generator.set_sign_flip_c00(gGlobals.sign_flip_c00);
        generator.set_sign_flip_c01(gGlobals.sign_flip_c01);
        generator.set_sign_flip_c10(gGlobals.sign_flip_c10);
        generator.set_sign_flip_c11(gGlobals.sign_flip_c11);
        generator.set_sign_flip_stec_residuals(gGlobals.sign_flip_stec_residuals);

        generator.set_gps_supported(gGlobals.generate_gps);
        generator.set_glonass_supported(gGlobals.generate_glonass);
        generator.set_galileo_supported(gGlobals.generate_galileo);
        generator.set_beidou_supported(gGlobals.generate_beidou);

        generator.set_flip_grid_bitmask(gGlobals.flip_grid_bitmask);
        generator.set_flip_orbit_correction(gGlobals.flip_orbit_correction);

        generator.set_generate_gad(gGlobals.generate_gad);
        generator.set_generate_ocb(gGlobals.generate_ocb);
        generator.set_generate_hpac(gGlobals.generate_hpac);
    }
#endif

#ifdef INCLUDE_GENERATOR_RTCM
    if (gGlobals.format == ssr_example::Format::LRF_UPER) {
        gStream.add_inspector<LppRtcmFramedOutput>(output_options, gGlobals.lrf_rtcm_id);
    }
#endif

#ifdef INCLUDE_GENERATOR_TOKORO
    if (gGlobals.format == ssr_example::Format::TOKORO) {
        tokoro_initialize(gStream, output_options);
    }
#endif

    LPP_Client::AD_Request request;
    LPP_Client             client{false /* experimental segmentation support */};
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

            if (location_server_options.skip_request_assistance_data) return;

            if (gGlobals.cell != new_cell) {
                INFOF("cell: %ld:%ld:%ld:%llu\n", new_cell.mcc, new_cell.mnc, new_cell.tac,
                      new_cell.cell);
                gGlobals.cell = new_cell;
                client.update_assistance_data(request, gGlobals.cell);
            } else {
                INFOF("cell: %ld:%ld:%ld:%llu (unchanged)\n", new_cell.mcc, new_cell.mnc,
                      new_cell.tac, new_cell.cell);
            }
        };

        ctrl_events->on_identity_imsi = [&](format::ctrl::IdentityImsi const& identity) {
            INFOF("identity: (imsi) %" PRIu64 "\n", identity.imsi());
            if (client_got_identity) return;
            client.set_identity_imsi(identity.imsi());
            client_got_identity = true;
        };
    }

    if (identity_options.wait_for_identity) {
        INFOF("waiting for identity\n");
        if (identity_options.imsi || identity_options.msisdn || identity_options.ipv4) {
            WARNF("  specified identity will be ignored");
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

    if (location_information_options.fake_location_info) {
        INFOF("using simulated location information");
        client.provide_location_information_callback(&location_information_options,
                                                     provide_location_information_callback_fake);
    } else {
        INFOF("using real location information");
        client.provide_location_information_callback(
            nullptr, provide_location_information_callback_streamline);
    }

    if (location_information_options.force) {
        INFOF("forcing location information");
        client.force_location_information();
    }

    client.set_update_rate(location_information_options.update_rate);
    if (location_information_options.unlock_update_rate) {
        INFOF("using requested update rate");
        client.unlock_update_rate();
    }

    if (!location_server_options.skip_request_assistance_data) {
        if (!client.connect(location_server_options.host.c_str(), location_server_options.port,
                            location_server_options.ssl, gGlobals.cell)) {
            ERRORF("unable to connect to location server");
            throw std::runtime_error("Unable to connect to location server");
        }

        request =
            client.request_assistance_data_ssr(gGlobals.cell, nullptr, assistance_data_callback);
        if (request == AD_REQUEST_INVALID) {
            ERRORF("unable to request assistance data");
            throw std::runtime_error("Unable to request assistance data");
        }

        client_initialized = true;
    }

    for (;;) {
        scheduler.execute_timeout(
            std::chrono::milliseconds(location_information_options.update_rate));

        if (!location_server_options.skip_request_assistance_data) {
            // client.process() MUST be called at least once every second, otherwise
            // ProvideLocationInformation messages will not be send to the server.
            if (!client.process()) {
                throw std::runtime_error("Unable to process LPP client (probably disconnected)");
            }
        }
    }
}

static bool assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message* message, void*) {
    gStream.push(LppMessage{message});
    return true;
}

namespace ssr_example {

void SsrCommand::parse(args::Subparser& parser) {
    // NOTE: parse may be called multiple times
    cleanup();

    mFormatArg = new args::ValueFlag<std::string>(parser, "format", "Format of the output",
                                                  {"format"}, args::Options::Single);
    mFormatArg->HelpDefault("xer");
    mFormatArg->HelpChoices({
        "xer",
        "asn1-uper",
#ifdef INCLUDE_GENERATOR_SPARTN
        "spartn",
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
        "tokoro",
#endif
#ifdef INCLUDE_GENERATOR_RTCM
        "lrf-uper",
#endif
    });

    mLRFMessageIdArg =
        new args::ValueFlag<int>(parser, "lrf-message-id", "RTCM message ID for LRF-UPER format",
                                 {"lrf-message-id"}, args::Options::Single);
    mLRFMessageIdArg->HelpDefault("355");

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
        new args::Flag(parser, "ublox-clock-correction",
                       "DEPRECATED: Flip the clock correction sign", {"ublox-clock-correction"});
    mNoUbloxClockCorrectionArg =
        new args::Flag(parser, "no-ublox-clock-correction", "Flip the clock correction sign",
                       {"no-ublox-clock-correction"});

    mForceContinuityArg =
        new args::Flag(parser, "force-continuity",
                       "DEPRECATED: Force SF022 (IODE Continuity) to 320s", {"force-continuity"});
    mNoForceContinuityArg =
        new args::Flag(parser, "no-force-continuity",
                       "Do not force SF022 (IODE Continuity) to 320s", {"no-force-continuity"});

    mAverageZenithDelayArg = new args::Flag(
        parser, "average-zenith-delay",
        "DEPRECATED: Compute T00 constant as the average zenith delay", {"average-zenith-delay"});
    mNoAverageZenithDelayArg = new args::Flag(
        parser, "no-average-zenith-delay",
        "Do not compute T00 constant as the average zenith delay", {"no-average-zenith-delay"});

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
    mFilterByResiduals = new args::Flag(
        parser, "filter-by-residuals",
        "Only include ionospheric residual satellites that have residuals for all grid points",
        {"filter-by-residuals"});
    mFilterByOcb = new args::Flag(
        parser, "filter-by-ocb",
        "Only include ionospheric residual satellites that also have OCB corrections",
        {"filter-by-ocb"});
    mIgnoreL2L = new args::Flag(parser, "ignore-l2l", "Ignore L2L biases", {"ignore-l2l"});

    mPrintRTCMArg =
        new args::Flag(parser, "print_rtcm", "Print RTCM messages info (only used for LRF-UPER)",
                       {"rtcm-print"}, args::Options::Single);

    mCodeBiasNoTranslateArg =
        new args::Flag(parser, "no-code-bias-translate", "Do not translate between code biases",
                       {"no-code-bias-translate"}, args::Options::Single);
    mCodeBiasNoCorrectionShiftArg =
        new args::Flag(parser, "no-code-bias-correction-shift",
                       "Do not apply correction shift to code biases when translating",
                       {"no-code-bias-correction-shift"}, args::Options::Single);
    mPhaseBiasNoTranslateArg =
        new args::Flag(parser, "no-phase-bias-translate", "Do not translate between phase biases",
                       {"no-phase-bias-translate"}, args::Options::Single);
    mPhaseBiasNoCorrectionShiftArg =
        new args::Flag(parser, "no-phase-bias-correction-shift",
                       "Do not apply correction shift to phase biases when translating",
                       {"no-phase-bias-correction-shift"}, args::Options::Single);
    mHydrostaticInZenithArg = new args::Flag(
        parser, "hydrostatic-in-zenith",
        "Use the remaning hydrostatic delay residual in the per grid-point zenith residual",
        {"hydrostatic-in-zenith"}, args::Options::Single);

    mStecMethod = new args::ValueFlag<std::string>(parser, "stec-method",
                                                   "STEC method to use for the polynomial",
                                                   {"stec-method"}, args::Options::Single);
    mStecMethod->HelpChoices({
        "default",
        "discard",
        "residual",
    });
    mStecMethod->HelpDefault("default");

    mNoStecTransform =
        new args::Flag(parser, "no-stec-transform", "Skip transforming the STEC from LPP to SPARTN",
                       {"no-stec-transform"});

    mStecInvalidToZero =
        new args::Flag(parser, "stec-invalid-to-zero",
                       "Set STEC values that would be invalid in SPARTN to zero instead",
                       {"stec-invalid-to-zero"});

    mSignFlipC00 =
        new args::Flag(parser, "sf-c00", "Flip the sign of the C00 coefficient", {"sf-c00"});
    mSignFlipC01 =
        new args::Flag(parser, "sf-c01", "Flip the sign of the C01 coefficient", {"sf-c01"});
    mSignFlipC10 =
        new args::Flag(parser, "sf-c10", "Flip the sign of the C10 coefficient", {"sf-c10"});
    mSignFlipC11 =
        new args::Flag(parser, "sf-c11", "Flip the sign of the C11 coefficient", {"sf-c11"});
    mSignFlipStecResiduals = new args::Flag(
        parser, "sf-stec-residuals", "Flip the sign of the STEC residuals", {"sf-stec-residuals"});

    mNoGPS = new args::Flag(parser, "no-gps", "Skip generating GPS SPARTN messages", {"no-gps"});
    mNoGLONASS = new args::Flag(parser, "no-glonass", "Skip generating GLONASS SPARTN messages",
                                {"no-glonass"});
    mNoGalileo = new args::Flag(parser, "no-galileo", "Skip generating Galileo SPARTN messages",
                                {"no-galileo"});
    mBeiDou    = new args::Flag(parser, "beidou", "Generate BeiDou SPARTN messages", {"beidou"});

    mFlipGridBitmask =
        new args::Flag(parser, "flip-grid-bitmask",
                       "Flip the grid bitmask for incoming LPP messages", {"flip-grid-bitmask"});

    mNoGenerateGAD  = new args::Flag(parser, "no-generate-gad", "Skip generating GAD messages",
                                     {"no-generate-gad"});
    mNoGenerateOCB  = new args::Flag(parser, "no-generate-ocb", "Skip generating OCB messages",
                                     {"no-generate-ocb"});
    mNoGenerateHPAC = new args::Flag(parser, "no-generate-hpac", "Skip generating HPAC messages",
                                     {"no-generate-hpac"});

    mFlipOrbitCorrection =
        new args::Flag(parser, "flip-orbit-correction", "Flip the sign of the orbit correction",
                       {"flip-orbit-correction"});
}

void SsrCommand::execute(Options options) {
    gGlobals.options                     = std::move(options);
    gGlobals.cell                        = {};
    gGlobals.format                      = ssr_example::Format::XER;
    gGlobals.lrf_rtcm_id                 = 355;
    gGlobals.ura_override                = -1;
    gGlobals.ura_default                 = -1;
    gGlobals.ublox_clock_correction      = true;
    gGlobals.force_continuity            = true;
    gGlobals.average_zenith_delay        = true;
    gGlobals.iode_shift                  = false;
    gGlobals.sf055_override              = -1;
    gGlobals.sf055_default               = -1;
    gGlobals.sf042_override              = -1;
    gGlobals.sf042_default               = -1;
    gGlobals.increasing_siou             = false;
    gGlobals.filter_by_residuals         = false;
    gGlobals.filter_by_ocb               = false;
    gGlobals.ignore_l2l                  = false;
    gGlobals.print_rtcm                  = false;
    gGlobals.hydrostatic_in_zenith       = false;
    gGlobals.stec_method                 = generator::spartn::StecMethod::Default;
    gGlobals.stec_transform              = true;
    gGlobals.stec_invalid_to_zero        = false;
    gGlobals.sign_flip_c00               = false;
    gGlobals.sign_flip_c01               = false;
    gGlobals.sign_flip_c10               = false;
    gGlobals.sign_flip_c11               = false;
    gGlobals.sign_flip_stec_residuals    = false;
    gGlobals.code_bias_translate         = true;
    gGlobals.code_bias_correction_shift  = true;
    gGlobals.phase_bias_translate        = true;
    gGlobals.phase_bias_correction_shift = true;
    gGlobals.generate_gps                = true;
    gGlobals.generate_glonass            = true;
    gGlobals.generate_galileo            = true;
    gGlobals.generate_beidou             = false;
    gGlobals.flip_grid_bitmask           = false;
    gGlobals.generate_gad                = true;
    gGlobals.generate_ocb                = true;
    gGlobals.generate_hpac               = true;
    gGlobals.flip_orbit_correction       = false;

    if (*mFormatArg) {
        if (mFormatArg->Get() == "xer") {
            gGlobals.format = ssr_example::Format::XER;
        } else if (mFormatArg->Get() == "asn1-uper") {
            gGlobals.format = ssr_example::Format::ASN1_UPER;
        }
#ifdef INCLUDE_GENERATOR_SPARTN
        else if (mFormatArg->Get() == "spartn") {
            gGlobals.format = ssr_example::Format::SPARTN_NEW;
        }
#endif
#ifdef INCLUDE_GENERATOR_RTCM
        else if (mFormatArg->Get() == "lrf-uper") {
            gGlobals.format = ssr_example::Format::LRF_UPER;
        }
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
        else if (mFormatArg->Get() == "tokoro") {
            gGlobals.format = ssr_example::Format::TOKORO;
        } else {
            throw args::ValidationError("Invalid format");
        }
#endif
    }

    if (*mLRFMessageIdArg) {
        gGlobals.lrf_rtcm_id = mLRFMessageIdArg->Get();
    }

    if (*mUraOverrideArg) {
        gGlobals.ura_override = mUraOverrideArg->Get();
    }

    if (*mUraDefaultArg) {
        gGlobals.ura_default = mUraDefaultArg->Get();
    }

    if (*mUbloxClockCorrectionArg) {
        printf("[DEPRECATED] ublox-clock-correction is deprecated, it is now true by default. Use "
               "no-ublox-clock-correction to disable\n");
    }

    if (*mNoUbloxClockCorrectionArg && mNoUbloxClockCorrectionArg->Get()) {
        gGlobals.ublox_clock_correction = false;
    }

    if (*mForceContinuityArg) {
        printf("[DEPRECATED] force-continuity is deprecated, it is now true by default. Use "
               "no-force-continuity to disable\n");
    }

    if (*mNoForceContinuityArg && mNoForceContinuityArg->Get()) {
        gGlobals.force_continuity = false;
    }

    if (*mAverageZenithDelayArg) {
        printf("[DEPRECATED] average-zenith-delay is deprecated, it is now true by default. Use "
               "no-average-zenith-delay to disable\n");
    }

    if (*mNoAverageZenithDelayArg && mNoAverageZenithDelayArg->Get()) {
        gGlobals.average_zenith_delay = false;
    }

    if (*mSf055Override) {
        gGlobals.sf055_override = mSf055Override->Get();
        if (gGlobals.sf055_override < 0) gGlobals.sf055_override = 0;
        if (gGlobals.sf055_override > 15) gGlobals.sf055_override = 15;
    }

    if (*mSf055Default) {
        gGlobals.sf055_default = mSf055Default->Get();
        if (gGlobals.sf055_default < 0) gGlobals.sf055_default = 0;
        if (gGlobals.sf055_default > 15) gGlobals.sf055_default = 15;
    }

    if (*mSf042Override) {
        gGlobals.sf042_override = mSf042Override->Get();
        if (gGlobals.sf042_override < 0) gGlobals.sf042_override = 0;
        if (gGlobals.sf042_override > 7) gGlobals.sf042_override = 7;
    }

    if (*mSf042Default) {
        gGlobals.sf042_default = mSf042Default->Get();
        if (gGlobals.sf042_default < 0) gGlobals.sf042_default = 0;
        if (gGlobals.sf042_default > 7) gGlobals.sf042_default = 7;
    }

    if (*mIncreasingSiou) {
        gGlobals.increasing_siou = mIncreasingSiou->Get();
    }

    if (*mFilterByResiduals) {
        gGlobals.filter_by_residuals = mFilterByResiduals->Get();
    }

    if (*mFilterByOcb) {
        gGlobals.filter_by_ocb = mFilterByOcb->Get();
    }

    if (*mIgnoreL2L) {
        gGlobals.ignore_l2l = mIgnoreL2L->Get();
    }

    if (*mPrintRTCMArg) {
        gGlobals.print_rtcm = true;
    }

    if (*mCodeBiasNoTranslateArg) {
        gGlobals.code_bias_translate = false;
    }

    if (*mCodeBiasNoCorrectionShiftArg) {
        gGlobals.code_bias_correction_shift = false;
    }

    if (*mPhaseBiasNoTranslateArg) {
        gGlobals.phase_bias_translate = false;
    }

    if (*mPhaseBiasNoCorrectionShiftArg) {
        gGlobals.phase_bias_correction_shift = false;
    }

    if (*mHydrostaticInZenithArg) {
        gGlobals.hydrostatic_in_zenith = true;
    }

    if (*mStecMethod) {
        if (mStecMethod->Get() == "default") {
            gGlobals.stec_method = generator::spartn::StecMethod::Default;
        } else if (mStecMethod->Get() == "discard") {
            gGlobals.stec_method = generator::spartn::StecMethod::DiscardC01C10C11;
        } else if (mStecMethod->Get() == "residual") {
            gGlobals.stec_method = generator::spartn::StecMethod::MoveToResiduals;
        } else {
            throw args::ValidationError("Invalid STEC method");
        }
    }

    if (*mNoStecTransform) {
        gGlobals.stec_transform = false;
    }

    if (*mStecInvalidToZero) {
        gGlobals.stec_invalid_to_zero = true;
    }

    if (*mSignFlipC00) {
        gGlobals.sign_flip_c00 = true;
    }

    if (*mSignFlipC01) {
        gGlobals.sign_flip_c01 = true;
    }

    if (*mSignFlipC10) {
        gGlobals.sign_flip_c10 = true;
    }

    if (*mSignFlipC11) {
        gGlobals.sign_flip_c11 = true;
    }

    if (*mSignFlipStecResiduals) {
        gGlobals.sign_flip_stec_residuals = true;
    }

    if (*mNoGPS) {
        gGlobals.generate_gps = false;
    }

    if (*mNoGLONASS) {
        gGlobals.generate_glonass = false;
    }

    if (*mNoGalileo) {
        gGlobals.generate_galileo = false;
    }

    if (*mBeiDou) {
        gGlobals.generate_beidou = true;
    }

    if (*mFlipGridBitmask) {
        gGlobals.flip_grid_bitmask = true;
    }

    if (*mNoGenerateGAD) {
        gGlobals.generate_gad = false;
    }

    if (*mNoGenerateOCB) {
        gGlobals.generate_ocb = false;
    }

    if (*mNoGenerateHPAC) {
        gGlobals.generate_hpac = false;
    }

    if (*mFlipOrbitCorrection) {
        gGlobals.flip_orbit_correction = true;
    }

    auto& cell_options  = gGlobals.options.cell_options;
    gGlobals.cell.mcc   = cell_options.mcc;
    gGlobals.cell.mnc   = cell_options.mnc;
    gGlobals.cell.tac   = cell_options.tac;
    gGlobals.cell.cell  = cell_options.cid;
    gGlobals.cell.is_nr = cell_options.is_nr;

    ::execute();
}

}  // namespace ssr_example
