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

struct SsrGlobals {
    Options                       options;
    ControlParser                 control_parser;
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

#ifdef INCLUDE_GENERATOR_SPARTN_OLD
static SPARTN_Generator gSpartnGeneratorOld;
#endif

#ifdef INCLUDE_GENERATOR_SPARTN
static generator::spartn::Generator gSpartnGeneratorNew;
#endif

static std::unique_ptr<UReceiver> gUbloxReceiver;
static std::unique_ptr<NReceiver> gNmeaReceiver;
static SsrGlobals                 gGlobals;

static void assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message*, void*);

[[noreturn]] void execute() {
    auto& cell_options                 = gGlobals.options.cell_options;
    auto& location_server_options      = gGlobals.options.location_server_options;
    auto& identity_options             = gGlobals.options.identity_options;
    auto& output_options               = gGlobals.options.output_options;
    auto& ublox_options                = gGlobals.options.ublox_options;
    auto& nmea_options                 = gGlobals.options.nmea_options;
    auto& location_information_options = gGlobals.options.location_information_options;
    auto& control_options              = gGlobals.options.control_options;

    gConvertConfidence95To68      = location_information_options.convert_confidence_95_to_68;
    gOutputEllipse68              = location_information_options.output_ellipse_68;
    gOverrideHorizontalConfidence = location_information_options.override_horizontal_confidence;

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
           cell_options.is_nr ? "[nr]" : "[lte]", gGlobals.cell.mcc, gGlobals.cell.mnc,
           gGlobals.cell.tac, gGlobals.cell.cell);

    for (auto& interface : output_options.interfaces) {
        interface->open();
        interface->print_info();
    }

    if (ublox_options.interface) {
        printf("[ublox]\n");
        ublox_options.interface->open();
        ublox_options.interface->print_info();

        if (!ublox_options.export_interfaces.empty()) {
            printf("[ublox-export]\n");
            for (auto& interface : ublox_options.export_interfaces) {
                interface->open();
                interface->print_info();
            }
        }

        gUbloxReceiver = std::unique_ptr<UReceiver>(new UReceiver(
            ublox_options.port, std::move(ublox_options.interface), ublox_options.print_messages,
            std::move(ublox_options.export_interfaces)));
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
    gSpartnGeneratorNew.set_ura_override(gGlobals.ura_override);
    gSpartnGeneratorNew.set_ura_default(gGlobals.ura_default);
    gSpartnGeneratorNew.set_ublox_clock_correction(gGlobals.ublox_clock_correction);
    if (gGlobals.force_continuity) {
        gSpartnGeneratorNew.set_continuity_indicator(320.0);
    }
    gSpartnGeneratorNew.set_compute_average_zenith_delay(gGlobals.average_zenith_delay);
    gSpartnGeneratorNew.set_iode_shift(gGlobals.iode_shift);

    if (gGlobals.sf055_override >= 0)
        gSpartnGeneratorNew.set_sf055_override(gGlobals.sf055_override);
    if (gGlobals.sf055_default >= 0) gSpartnGeneratorNew.set_sf055_default(gGlobals.sf055_default);
    if (gGlobals.sf042_override >= 0)
        gSpartnGeneratorNew.set_sf042_override(gGlobals.sf042_override);
    if (gGlobals.sf042_default >= 0) gSpartnGeneratorNew.set_sf042_default(gGlobals.sf042_default);

    gSpartnGeneratorNew.set_increasing_siou(gGlobals.increasing_siou);
    gSpartnGeneratorNew.set_filter_by_residuals(gGlobals.filter_by_residuals);
    gSpartnGeneratorNew.set_filter_by_ocb(gGlobals.filter_by_ocb);
    gSpartnGeneratorNew.set_ignore_l2l(gGlobals.ignore_l2l);
    gSpartnGeneratorNew.set_stec_invalid_to_zero(gGlobals.stec_invalid_to_zero);

    gSpartnGeneratorNew.set_code_bias_translate(gGlobals.code_bias_translate);
    gSpartnGeneratorNew.set_code_bias_correction_shift(gGlobals.code_bias_correction_shift);
    gSpartnGeneratorNew.set_phase_bias_translate(gGlobals.phase_bias_translate);
    gSpartnGeneratorNew.set_phase_bias_correction_shift(gGlobals.phase_bias_correction_shift);

    gSpartnGeneratorNew.set_hydrostatic_in_zenith(gGlobals.hydrostatic_in_zenith);
    gSpartnGeneratorNew.set_stec_method(gGlobals.stec_method);
    gSpartnGeneratorNew.set_stec_transform(gGlobals.stec_transform);
    gSpartnGeneratorNew.set_sign_flip_c00(gGlobals.sign_flip_c00);
    gSpartnGeneratorNew.set_sign_flip_c01(gGlobals.sign_flip_c01);
    gSpartnGeneratorNew.set_sign_flip_c10(gGlobals.sign_flip_c10);
    gSpartnGeneratorNew.set_sign_flip_c11(gGlobals.sign_flip_c11);
    gSpartnGeneratorNew.set_sign_flip_stec_residuals(gGlobals.sign_flip_stec_residuals);

    gSpartnGeneratorNew.set_gps_supported(gGlobals.generate_gps);
    gSpartnGeneratorNew.set_glonass_supported(gGlobals.generate_glonass);
    gSpartnGeneratorNew.set_galileo_supported(gGlobals.generate_galileo);
    gSpartnGeneratorNew.set_beidou_supported(gGlobals.generate_beidou);

    gSpartnGeneratorNew.set_flip_grid_bitmask(gGlobals.flip_grid_bitmask);
    gSpartnGeneratorNew.set_flip_orbit_correction(gGlobals.flip_orbit_correction);

    gSpartnGeneratorNew.set_generate_gad(gGlobals.generate_gad);
    gSpartnGeneratorNew.set_generate_ocb(gGlobals.generate_ocb);
    gSpartnGeneratorNew.set_generate_hpac(gGlobals.generate_hpac);
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

        gGlobals.control_parser.on_cid = [&](CellID cell) {
            if (!client_initialized) return;
            if (gGlobals.cell != cell) {
                printf("[control] cell: %ld:%ld:%ld:%llu\n", cell.mcc, cell.mnc, cell.tac,
                       cell.cell);
                gGlobals.cell = cell;
                client.update_assistance_data(request, gGlobals.cell);
            } else {
                printf("[control] cell: %ld:%ld:%ld:%llu (unchanged)\n", cell.mcc, cell.mnc,
                       cell.tac, cell.cell);
            }
        };

        gGlobals.control_parser.on_identity_imsi = [&](unsigned long long imsi) {
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

            gGlobals.control_parser.parse(control_options.interface);
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

    client.set_update_rate(location_information_options.update_rate);
    if (location_information_options.unlock_update_rate) {
        client.unlock_update_rate();
        printf("  unlock update rate: true\n");
    } else {
        printf("  unlock update rate: false\n");
    }

    if (!client.connect(location_server_options.host.c_str(), location_server_options.port,
                        location_server_options.ssl, gGlobals.cell)) {
        throw std::runtime_error("Unable to connect to location server");
    }

    request = client.request_assistance_data_ssr(gGlobals.cell, nullptr, assistance_data_callback);
    if (request == AD_REQUEST_INVALID) {
        throw std::runtime_error("Unable to request assistance data");
    }

    client_initialized = true;

    for (;;) {
        struct timespec timeout;
        timeout.tv_sec  = 0;
        timeout.tv_nsec = 1000000 * location_information_options.update_rate;
        nanosleep(&timeout, nullptr);

        // client.process() MUST be called at least once every second, otherwise
        // ProvideLocationInformation messages will not be send to the server.
        if (!client.process()) {
            throw std::runtime_error("Unable to process LPP client (probably disconnected)");
        }

        if (control_options.interface) {
            gGlobals.control_parser.parse(control_options.interface);
        }
    }
}

static void assistance_data_callback(LPP_Client* client, LPP_Transaction*, LPP_Message* message,
                                     void*) {
    if (gGlobals.format == ssr_example::Format::XER) {
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
        for (auto& interface : gGlobals.options.output_options.interfaces) {
            interface->write(xer_message.c_str(), xer_message.size());
        }
    } else if (gGlobals.format == ssr_example::Format::ASN1_UPER) {
        auto octet = client->encode(message);
        if (octet) {
            for (auto& interface : gGlobals.options.output_options.interfaces) {
                interface->write(octet->buf, octet->size);
            }

            ASN_STRUCT_FREE(asn_DEF_OCTET_STRING, octet);
        }
    }
#ifdef INCLUDE_GENERATOR_SPARTN_OLD
    else if (gGlobals.format == ssr_example::Format::SPARTN_OLD) {
        auto messages = gSpartnGeneratorOld.generate(message, gGlobals.ura_override,
                                                     gGlobals.ublox_clock_correction,
                                                     gGlobals.force_continuity);
        for (auto& msg : messages) {
            auto bytes = SPARTN_Transmitter::build(msg);
            for (auto& interface : gGlobals.options.output_options.interfaces) {
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
    else if (gGlobals.format == ssr_example::Format::SPARTN_NEW) {
        auto messages = gSpartnGeneratorNew.generate(message);
        for (auto& msg : messages) {
            auto data = msg.build();
            if (data.size() == 0) {
                printf("Size of SPARTN payload is above the 1024 byte limit\n");
                continue;
            }

            for (auto& interface : gGlobals.options.output_options.interfaces) {
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
    else if (gGlobals.format == ssr_example::Format::LRF_UPER) {
        auto octet = client->encode(message);
        if (octet) {
            auto submessages = generator::rtcm::Generator::generate_framing(
                gGlobals.lrf_rtcm_id, octet->buf, octet->size);

            if (gGlobals.print_rtcm) {
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
                for (auto& interface : gGlobals.options.output_options.interfaces) {
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
#ifdef INCLUDE_GENERATOR_SPARTN_OLD
        "spartn-old",
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
    gGlobals.control_parser              = {};
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
#ifdef INCLUDE_GENERATOR_SPARTN_OLD
        else if (mFormatArg->Get() == "spartn-old") {
            gGlobals.format = ssr_example::Format::SPARTN_OLD;
        }
#endif
#ifdef INCLUDE_GENERATOR_RTCM
        else if (mFormatArg->Get() == "lrf-uper") {
            gGlobals.format = ssr_example::Format::LRF_UPER;
        }
#endif
        else {
            throw args::ValidationError("Invalid format");
        }
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
