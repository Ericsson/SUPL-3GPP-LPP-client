#include "osr_example.h"
#include <lpp/location_information.h>
#include <lpp/lpp.h>
#include <receiver/nmea/threaded_receiver.hpp>
#include <receiver/ublox/threaded_receiver.hpp>
#include <sstream>
#include <stdexcept>

#include "control.hpp"
#include "location_information.h"

#ifdef INCLUDE_GENERATOR_RTCM
#include <generator/rtcm/generator.hpp>
using RtcmGenerator = std::unique_ptr<generator::rtcm::Generator>;
static RtcmGenerator gGenerator;
#endif

using UReceiver = receiver::ublox::ThreadedReceiver;
using NReceiver = receiver::nmea::ThreadedReceiver;

static CellID                         gCell;
static osr_example::Format            gFormat;
static generator::rtcm::MessageFilter gFilter;
static Options                        gOptions;
static bool                           gPrintRtcm;
static ControlParser                  gControlParser;

static std::unique_ptr<UReceiver> gUbloxReceiver;
static std::unique_ptr<NReceiver> gNmeaReceiver;

static void assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message*, void*);

[[noreturn]] void execute(Options options, osr_example::Format format,
                          osr_example::MsmType msm_type, bool print_rtcm) {
    gOptions   = std::move(options);
    gFormat    = format;
    gPrintRtcm = print_rtcm;

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

#ifdef INCLUDE_GENERATOR_RTCM
    // Enable generation of message for GPS, GLONASS, Galileo, and Beidou.
    gFilter                 = generator::rtcm::MessageFilter{};
    gFilter.systems.gps     = true;
    gFilter.systems.glonass = true;
    gFilter.systems.galileo = true;
    gFilter.systems.beidou  = true;

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
#endif

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

#ifdef INCLUDE_GENERATOR_RTCM
    // Create RTCM generator for converting LPP messages to RTCM messages.
    gGenerator = std::unique_ptr<generator::rtcm::Generator>(new generator::rtcm::Generator());
#endif

    LPP_Client::AD_Request request;
    LPP_Client             client{false /* enable experimental segmentation support */};
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

    // Request OSR assistance data from location server for the 'cell' and register a callback
    // that will be called when we receive assistance data.
    request = client.request_assistance_data(gCell, nullptr, assistance_data_callback);
    if (request == AD_REQUEST_INVALID) {
        throw std::runtime_error("Unable to request assistance data");
    }

    client_initialized = true;

    for (;;) {
        struct timespec timeout;
        timeout.tv_sec  = 0;
        timeout.tv_nsec = 1000000 * 100;  // 100 ms
        nanosleep(&timeout, nullptr);

        if (control_options.interface) {
            gControlParser.parse(control_options.interface);
        }

        // client.process() MUST be called at least once every second, otherwise
        // ProvideLocationInformation messages will not be send to the server.
        if (!client.process()) {
            throw std::runtime_error("Unable to process LPP client (probably disconnected)");
        }
    }
}

static void transmit(void const* buffer, size_t size) {
    for (auto& interface : gOptions.output_options.interfaces) {
        interface->write(buffer, size);
    }
}

static void assistance_data_callback(LPP_Client* client, LPP_Transaction*, LPP_Message* message,
                                     void*) {
    if (gFormat == osr_example::Format::XER) {
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
        transmit(xer_message.data(), xer_message.size());
    } else if (gFormat == osr_example::Format::ASN1_UPER) {
        auto octet = client->encode(message);
        if (octet) {
            transmit(octet->buf, octet->size);

            ASN_STRUCT_FREE(asn_DEF_OCTET_STRING, octet);
        }
    }
#ifdef INCLUDE_GENERATOR_RTCM
    else if (gFormat == osr_example::Format::LRF_UPER) {
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
                transmit(buffer, size);
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
    } else if (gFormat == osr_example::Format::RTCM) {
        auto messages = gGenerator->generate(message, gFilter);

        if (gPrintRtcm) {
            size_t length = 0;
            for (auto& submessage : messages) {
                length += submessage.data().size();
            }

            printf("RTCM: %4zu bytes | ", length);
            for (auto& submessage : messages) {
                printf("%4i ", submessage.id());
            }
            printf("\n");
        }

        for (auto& submessage : messages) {
            auto buffer = submessage.data().data();
            auto size   = submessage.data().size();
            transmit(buffer, size);
        }

        if (gUbloxReceiver) {
            auto interface = gUbloxReceiver->interface();
            if (interface) {
                for (auto& submessage : messages) {
                    auto buffer = submessage.data().data();
                    auto size   = submessage.data().size();
                    interface->write(buffer, size);
                }
            }
        } else if (gNmeaReceiver) {
            auto interface = gNmeaReceiver->interface();
            if (interface) {
                for (auto& submessage : messages) {
                    auto buffer = submessage.data().data();
                    auto size   = submessage.data().size();
                    interface->write(buffer, size);
                }
            }
        }
    }
#endif
    else {
        throw std::runtime_error("Unsupported format");
    }
}

namespace osr_example {

void OsrCommand::parse(args::Subparser& parser) {
    // NOTE: parse may be called multiple times
    delete mFormatArg;
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
    });

    mMsmTypeArg = new args::ValueFlag<std::string>(parser, "msm_type", "RTCM MSM type",
                                                   {'y', "msm_type"}, args::Options::Single);
    mMsmTypeArg->HelpDefault("any");
    mMsmTypeArg->HelpChoices({"any", "4", "5", "6", "7"});

    // the default value is true, thus this is a negated flag
    mPrintRTCMArg = new args::Flag(parser, "print_rtcm", "Do not print RTCM messages info",
                                   {"rtcm-print"}, args::Options::Single);
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

    ::execute(std::move(options), format, msm_type, print_rtcm);
}

}  // namespace osr_example
