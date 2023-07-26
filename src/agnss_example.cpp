#include "agnss_example.h"
#include <lpp/lpp.h>
#include <sstream>
#include <stdexcept>
#include <transmitter/transmitter.h>
#include <utility/types.h>

namespace agnss_example {

static Format      gFormat;
static CellID      gCell;
static Transmitter gTransmitter;

static void agnss_assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message*, void*);

void execute(const LocationServerOptions& location_server_options,
             const IdentityOptions& identity_options, const CellOptions& cell_options,
             UNUSED const ModemOptions& modem_options, const OutputOptions& output_options,
             Format format) {
    gFormat = format;
    gCell   = CellID{
          .mcc  = cell_options.mcc,
          .mnc  = cell_options.mnc,
          .tac  = cell_options.tac,
          .cell = cell_options.cid,
    };

    printf("[settings]\n");
    printf("  location server:    \"%s:%d\" %s\n", location_server_options.host.c_str(),
           location_server_options.port, location_server_options.ssl ? "[ssl]" : "");
    printf("  identity:           ");
    if (identity_options.imsi) printf("imsi: %lu\n", *identity_options.imsi);
    if (identity_options.msisdn) printf("msisdn: %lu\n", *identity_options.msisdn);
    if (identity_options.ipv4) printf("ipv4: %s\n", identity_options.ipv4->c_str());
    printf("  cell information:   %ld:%ld:%ld:%ld (mcc:mnc:tac:id)\n", gCell.mcc, gCell.mnc,
           gCell.tac, gCell.cell);

    if (output_options.file)
        gTransmitter.add_file_target(output_options.file->file_path, true /* truncate */);
    if (output_options.serial)
        gTransmitter.add_serial_target(output_options.serial->device,
                                       output_options.serial->baud_rate);
    if (output_options.i2c)
        gTransmitter.add_i2c_target(output_options.i2c->device, output_options.i2c->address);
    if (output_options.tcp)
        gTransmitter.add_tcp_target(output_options.tcp->ip_address, output_options.tcp->port);
    if (output_options.udp)
        gTransmitter.add_udp_target(output_options.udp->ip_address, output_options.udp->port);
    if (output_options.stdout_output) gTransmitter.add_stdout_target();

    LPP_Client client{false /* enable experimental segmentation support */};
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
    if (gFormat == Format::XER) {
        std::stringstream buffer;
        xer_encode(
            &asn_DEF_LPP_Message, message, XER_F_BASIC,
            [](const void* buffer, size_t size, void* app_key) -> int {
                auto stream = static_cast<std::ostream*>(app_key);
                stream->write(static_cast<const char*>(buffer), size);
                return 0;
            },
            &buffer);
        gTransmitter.send(buffer.str().c_str(), buffer.str().size());
    } else {
        throw std::runtime_error("Unsupported format");
    }
}

}  // namespace agnss_example
