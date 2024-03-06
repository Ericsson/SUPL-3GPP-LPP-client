#include "ssr_example.h"
#include <generator/spartn/generator.h>
#include <generator/spartn/transmitter.h>
#include <generator/spartn2/generator.hpp>
#include <iostream>
#include <lpp/location_information.h>
#include <lpp/lpp.h>
#include <modem.h>
#include <receiver/nmea/threaded_receiver.hpp>
#include <receiver/ublox/threaded_receiver.hpp>
#include <sstream>
#include <stdexcept>
#include "location_information.h"

using UReceiver = receiver::ublox::ThreadedReceiver;
using NReceiver = receiver::nmea::ThreadedReceiver;

static CellID                       gCell;
static ssr_example::Format          gFormat;
static int                          gUraOverride;
static bool                         gUBloxClockCorrection;
static bool                         gForceIodeContinuity;
static bool                         gAverageZenithDelay;
static bool                         gEnableIodeShift;
static Options                      gOptions;
static SPARTN_Generator             gSpartnGeneratorOld;
static generator::spartn::Generator gSpartnGeneratorNew;

static std::unique_ptr<Modem_AT>  gModem;
static std::unique_ptr<UReceiver> gUbloxReceiver;
static std::unique_ptr<NReceiver> gNmeaReceiver;

static void assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message*, void*);

void execute(Options options, ssr_example::Format format, int ura_override,
             bool ublox_clock_correction, bool force_continuity, bool average_zenith_delay,
             bool enable_iode_shift) {
    gOptions              = std::move(options);
    gFormat               = format;
    gUraOverride          = ura_override;
    gUBloxClockCorrection = ublox_clock_correction;
    gForceIodeContinuity  = force_continuity;
    gAverageZenithDelay   = average_zenith_delay;
    gEnableIodeShift      = enable_iode_shift;

    auto& cell_options                 = gOptions.cell_options;
    auto& location_server_options      = gOptions.location_server_options;
    auto& identity_options             = gOptions.identity_options;
    auto& modem_options                = gOptions.modem_options;
    auto& output_options               = gOptions.output_options;
    auto& ublox_options                = gOptions.ublox_options;
    auto& nmea_options                 = gOptions.nmea_options;
    auto& location_information_options = gOptions.location_information_options;

    gCell = CellID{
        .mcc  = cell_options.mcc,
        .mnc  = cell_options.mnc,
        .tac  = cell_options.tac,
        .cell = cell_options.cid,
    };

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

    if (modem_options.device) {
        gModem = std::unique_ptr<Modem_AT>(
            new Modem_AT(modem_options.device->device, modem_options.device->baud_rate, gCell));
        if (!gModem->initialize()) {
            throw std::runtime_error("Unable to initialize modem");
        }
    }

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

    gSpartnGeneratorNew.set_ura_override(gUraOverride);
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

    LPP_Client client{false /* experimental segmentation support */};

    if (!identity_options.use_supl_identity_fix) {
        client.use_incorrect_supl_identity();
    }

    if (identity_options.imsi) {
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

    client.provide_ecid_callback(gModem.get(), provide_ecid_callback);

    if (!client.connect(location_server_options.host.c_str(), location_server_options.port,
                        location_server_options.ssl, gCell)) {
        throw std::runtime_error("Unable to connect to location server");
    }

    auto request = client.request_assistance_data_ssr(gCell, NULL, assistance_data_callback);
    if (request == AD_REQUEST_INVALID) {
        throw std::runtime_error("Unable to request assistance data");
    }

    for (;;) {
        struct timespec timeout;
        timeout.tv_sec  = 0;
        timeout.tv_nsec = 1000000 * 100;  // 100 ms
        nanosleep(&timeout, NULL);

        // client.process() MUST be called at least once every second, otherwise
        // ProvideLocationInformation messages will not be send to the server.
        if (!client.process()) {
            throw std::runtime_error("Unable to process LPP client (probably disconnected)");
        }
    }
}

static void assistance_data_callback(LPP_Client* client, LPP_Transaction*, LPP_Message* message,
                                     void*) {
    if (gFormat == ssr_example::Format::SPARTN_OLD) {
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
    } else if (gFormat == ssr_example::Format::SPARTN_NEW) {
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
    } else if (gFormat == ssr_example::Format::XER) {
        std::stringstream buffer;
        xer_encode(
            &asn_DEF_LPP_Message, message, XER_F_BASIC,
            [](const void* buffer, size_t size, void* app_key) -> int {
                auto stream = static_cast<std::ostream*>(app_key);
                stream->write(static_cast<const char*>(buffer), size);
                return 0;
            },
            &buffer);
        auto message = buffer.str();
        for (auto& interface : gOptions.output_options.interfaces) {
            interface->write(message.c_str(), message.size());
        }
    } else if (gFormat == ssr_example::Format::ASN1_UPER) {
        auto octet = client->encode(message);
        if (octet) {
            for (auto& interface : gOptions.output_options.interfaces) {
                interface->write(octet->buf, octet->size);
            }

            ASN_STRUCT_FREE(asn_DEF_OCTET_STRING, octet);
        }
    } else {
        throw std::runtime_error("Unsupported format");
    }
}

namespace ssr_example {

void SsrCommand::parse(args::Subparser& parser) {
    // NOTE: parse may be called multiple times
    delete mFormatArg;
    delete mUraOverrideArg;
    delete mUbloxClockCorrectionArg;
    delete mForceContinuityArg;
    delete mAverageZenithDelayArg;
    delete mEnableIodeShift;

    mFormatArg = new args::ValueFlag<std::string>(parser, "format", "Format of the output",
                                                  {"format"}, args::Options::Single);
    mFormatArg->HelpDefault("xer");
    mFormatArg->HelpChoices({"xer", "spartn", "spartn-old", "asn1-uper"});

    mUraOverrideArg = new args::ValueFlag<int>(
        parser, "ura",
        "A hacky fix to set a nominal value for the URA if the LPP message does not include it"
        ", value will be clamped between 0-7.",
        {"ura-override"}, args::Options::Single);
    mUraOverrideArg->HelpDefault("0");

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
}

void SsrCommand::execute(Options options) {
    auto format = ssr_example::Format::XER;
    if (*mFormatArg) {
        if (mFormatArg->Get() == "xer") {
            format = ssr_example::Format::XER;
        } else if (mFormatArg->Get() == "spartn") {
            format = ssr_example::Format::SPARTN_NEW;
        } else if (mFormatArg->Get() == "spartn-old") {
            format = ssr_example::Format::SPARTN_OLD;
        } else if (mFormatArg->Get() == "asn1-uper") {
            format = ssr_example::Format::ASN1_UPER;
        } else {
            throw args::ValidationError("Invalid format");
        }
    }

    auto ura_override = 0;
    if (*mUraOverrideArg) {
        ura_override = mUraOverrideArg->Get();
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

    ::execute(std::move(options), format, ura_override, ublox_clock_correction, force_continuity,
              average_zenith_delay, iode_shift);
}

}  // namespace ssr_example
