#include "osr_example.h"
#include <generator/rtcm/generator.hpp>
#include <lpp/location_information.h>
#include <lpp/lpp.h>
#include <modem.h>
#include <rtcm_generator.h>
#include <sstream>
#include <stdexcept>
#include <transmitter/transmitter.h>
#include "location_information.h"

namespace osr_example {

static CellID                                      gCell;
static std::unique_ptr<Modem_AT>                   gModem;
static Transmitter                                 gTransmitter;
static std::unique_ptr<RTCMGenerator>              gGenerator;
static Format                                      gFormat;
static std::unique_ptr<generator::rtcm::Generator> gGenerator2;

static void osr_assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message*, void*);

void execute(const LocationServerOptions& location_server_options,
             const IdentityOptions& identity_options, const CellOptions& cell_options,
             const ModemOptions& modem_options, const OutputOptions& output_options, Format format,
             MsmType msm_type) {
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

    if (modem_options.device) {
        gModem = std::unique_ptr<Modem_AT>(
            new Modem_AT(modem_options.device->device, modem_options.device->baud_rate, gCell));
        if (!gModem->initialize()) {
            throw std::runtime_error("Unable to initialize modem");
        }
    }

    LPP_Client client{false /* enable experimental segmentation support */};

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

    // Enable generation of message for GPS, GLONASS, Galileo, and Beidou.
    auto gnss = GNSSSystems{
        .gps     = true,
        .glonass = true,
        .galileo = true,
        .beidou  = true,
    };

    printf("  gnss support:      ");
    if (gnss.gps) printf(" GPS");
    if (gnss.glonass) printf(" GLO");
    if (gnss.galileo) printf(" GAL");
    if (gnss.beidou) printf(" BDS");
    printf("\n");

    // Filter what messages you want generated. (MSM6 is not supported)
    auto filter = MessageFilter{};
    if (msm_type == MsmType::MSM4) {
        filter.msm.msm4 = true;
        filter.msm.msm5 = false;
        filter.msm.msm7 = false;
    } else if (msm_type == MsmType::MSM5) {
        filter.msm.msm4 = false;
        filter.msm.msm5 = true;
        filter.msm.msm7 = false;
    } else if (msm_type == MsmType::MSM7) {
        filter.msm.msm4 = false;
        filter.msm.msm5 = false;
        filter.msm.msm7 = true;
    }

    printf("  msm support:       ");
    if (filter.msm.msm4) printf(" MSM4");
    if (filter.msm.msm5) printf(" MSM5");
    if (filter.msm.msm7) printf(" MSM7");
    printf("\n");

    // Create RTCM generator for converting LPP messages to RTCM messages.
    gGenerator  = std::unique_ptr<RTCMGenerator>(new RTCMGenerator{gnss, filter});
    gGenerator2 = std::unique_ptr<generator::rtcm::Generator>(new generator::rtcm::Generator());

    // Request OSR assistance data from location server for the 'cell' and register a callback
    // that will be called when we receive assistance data.
    LPP_Client::AD_Request request =
        client.request_assistance_data(gCell, NULL, osr_assistance_data_callback);
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

static void osr_assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message* message,
                                         void*) {
    if (gFormat == Format::RTCM) {
        if (!gGenerator->process(message)) {
            // Segmented LPP message, waiting for rest before converting to RTCM.
            return;
        }

        // Convert LPP messages to buffer of RTCM messages.
        Generated     generated_messages{};
        unsigned char buffer[4 * 4096];
        auto          size   = sizeof(buffer);
        auto          length = gGenerator->convert(buffer, &size, &generated_messages);

        printf("[OSR] length: %4zu | msm%i | ", length, generated_messages.msm);
        if (generated_messages.mt1074) printf("1074 ");
        if (generated_messages.mt1075) printf("1075 ");
        if (generated_messages.mt1076) printf("1076 ");
        if (generated_messages.mt1077) printf("1077 ");
        if (generated_messages.mt1084) printf("1084 ");
        if (generated_messages.mt1085) printf("1085 ");
        if (generated_messages.mt1086) printf("1086 ");
        if (generated_messages.mt1087) printf("1087 ");
        if (generated_messages.mt1094) printf("1094 ");
        if (generated_messages.mt1095) printf("1095 ");
        if (generated_messages.mt1096) printf("1096 ");
        if (generated_messages.mt1097) printf("1097 ");
        if (generated_messages.mt1124) printf("1124 ");
        if (generated_messages.mt1125) printf("1125 ");
        if (generated_messages.mt1126) printf("1126 ");
        if (generated_messages.mt1127) printf("1127 ");
        if (generated_messages.mt1030) printf("1030 ");
        if (generated_messages.mt1031) printf("1031 ");
        if (generated_messages.mt1230) printf("1230 ");
        if (generated_messages.mt1006) printf("1006 ");
        if (generated_messages.mt1032) printf("1032 ");
        printf("\n");
        if (length > 0) {
            gTransmitter.send(buffer, length);
        }
    } else if (gFormat == Format::RG2) {
        auto filter   = generator::rtcm::MessageFilter{};
        filter.systems.beidou = true;
        filter.systems.galileo = true;
        filter.systems.glonass = true;
        auto messages = gGenerator2->generate(message, filter);

        for (auto& message : messages) {
            printf("[OSR] length: %4zu | id: %04u\n", message.data().size(), message.id());
            gTransmitter.send(message.data().data(), message.data().size());
        }
    } else if (gFormat == Format::XER) {
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

}  // namespace osr_example
