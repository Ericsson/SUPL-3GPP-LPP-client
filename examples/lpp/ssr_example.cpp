#include "ssr_example.h"
#include <iostream>
#include <lpp/location_information.h>
#include <lpp/lpp.h>
#include <modem.h>
#include <sstream>
#include <stdexcept>
#include "location_information.h"

static CellID                    gCell;
static std::unique_ptr<Modem_AT> gModem;
static ssr_example::Format       gFormat;
static int                       gUraOverride;
static bool                      gUBloxClockCorrection;
static bool                      gForceIodeContinuity;
static Options                   gOptions;

static void assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message*, void*);

void execute(Options options, ssr_example::Format format, int ura_override,
             bool ublox_clock_correction, bool force_continuity) {
    gOptions              = std::move(options);
    gFormat               = format;
    gUraOverride          = ura_override;
    gUBloxClockCorrection = ublox_clock_correction;
    gForceIodeContinuity  = force_continuity;

    auto& cell_options            = gOptions.cell_options;
    auto& location_server_options = gOptions.location_server_options;
    auto& identity_options        = gOptions.identity_options;
    auto& modem_options           = gOptions.modem_options;
    auto& output_options          = gOptions.output_options;

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
        printf("imsi: %lu\n", *identity_options.imsi);
    else if (identity_options.msisdn)
        printf("msisdn: %lu\n", *identity_options.msisdn);
    else if (identity_options.ipv4)
        printf("ipv4: %s\n", identity_options.ipv4->c_str());
    else
        printf("none\n");
    printf("  cell information:   %ld:%ld:%ld:%ld (mcc:mnc:tac:id)\n", gCell.mcc, gCell.mnc,
           gCell.tac, gCell.cell);

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

    LPP_Client client{false /* experimental segmentation support */};

    if (identity_options.imsi) {
        client.set_identity_imsi(*identity_options.imsi);
    } else if (identity_options.msisdn) {
        client.set_identity_msisdn(*identity_options.msisdn);
    } else if (identity_options.ipv4) {
        client.set_identity_ipv4(*identity_options.ipv4);
    } else {
        throw std::runtime_error("No identity provided");
    }

    client.provide_location_information_callback(NULL, provide_location_information_callback);
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

static void assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message* message, void*) {
    if (gFormat == ssr_example::Format::XER) {
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

    mFormatArg = new args::ValueFlag<std::string>(parser, "format", "Format of the output",
                                                  {"format"}, args::Options::Single);
    mFormatArg->HelpDefault("xer");
    mFormatArg->HelpChoices({"xer"});

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
}

void SsrCommand::execute(Options options) {
    auto format = ssr_example::Format::XER;
    if (*mFormatArg) {
        if (mFormatArg->Get() == "xer") {
            format = ssr_example::Format::XER;
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

    ::execute(std::move(options), format, ura_override, ublox_clock_correction, force_continuity);
}

}  // namespace ssr_example
