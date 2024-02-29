#include "agnss_example.h"
#include <lpp/lpp.h>
#include <sstream>
#include <stdexcept>
#include <utility/types.h>

static agnss_example::Format gFormat;
static CellID                gCell;
static Options               gOptions;

static void agnss_assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message*, void*);

void execute(Options options, agnss_example::Format format) {
    gOptions = std::move(options);
    gFormat  = format;

    auto& cell_options            = gOptions.cell_options;
    auto& location_server_options = gOptions.location_server_options;
    auto& identity_options        = gOptions.identity_options;
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
    if (identity_options.imsi) printf("imsi: %llu\n", *identity_options.imsi);
    if (identity_options.msisdn) printf("msisdn: %llu\n", *identity_options.msisdn);
    if (identity_options.ipv4) printf("ipv4: %s\n", identity_options.ipv4->c_str());
    printf("  cell information:   %s %ld:%ld:%ld:%llu (mcc:mnc:tac:id)\n",
           cell_options.is_nr ? "[nr]" : "[lte]", gCell.mcc, gCell.mnc, gCell.tac, gCell.cell);

    for (auto& interface : output_options.interfaces) {
        interface->open();
        interface->print_info();
    }

    LPP_Client client{false /* enable experimental segmentation support */};

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

    if (!client.connect(location_server_options.host.c_str(), location_server_options.port,
                        location_server_options.ssl, gCell)) {
        throw std::runtime_error("Unable to connect to location server");
    }

    LPP_Client::AD_Request request =
        client.request_agnss(gCell, NULL, agnss_assistance_data_callback);
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

static void agnss_assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message* message,
                                           void*) {
    if (gFormat == agnss_example::Format::XER) {
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

namespace agnss_example {

void AgnssCommand::parse(args::Subparser& parser) {
    // NOTE: parse may be called multiple times
    delete mFormatArg;

    mFormatArg = new args::ValueFlag<std::string>(parser, "format", "Format", {'f', "format"},
                                                  args::Options::Single);
    mFormatArg->HelpDefault("xer");
    mFormatArg->HelpChoices({"xer"});
}

void AgnssCommand::execute(Options options) {
    auto format = Format::XER;

    if (*mFormatArg) {
        if (mFormatArg->Get() == "xer") {
            format = Format::XER;
        } else {
            throw args::ValidationError("Invalid format");
        }
    }

    ::execute(std::move(options), format);
}

}  // namespace agnss_example
