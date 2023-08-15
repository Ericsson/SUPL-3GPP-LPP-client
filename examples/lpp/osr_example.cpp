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
static Format                                      gFormat;
static std::unique_ptr<generator::rtcm::Generator> gGenerator;
static generator::rtcm::MessageFilter              gFilter;
static std::vector<interface::Interface*>          gInterfaces;

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
    gInterfaces = output_options.interfaces;

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
    auto filter = MessageFilter{};
    switch (msm_type) {
    case MsmType::MSM4: gFilter.msm.force_msm4 = true; break;
    case MsmType::MSM5: gFilter.msm.force_msm5 = true; break;
    case MsmType::MSM6: gFilter.msm.force_msm6 = true; break;
    case MsmType::MSM7: gFilter.msm.force_msm7 = true; break;
    case MsmType::ANY: break;
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

    for(auto interface : gInterfaces) {
        interface->open();
        interface->print_info();
    }

    // Create RTCM generator for converting LPP messages to RTCM messages.
    gGenerator = std::unique_ptr<generator::rtcm::Generator>(new generator::rtcm::Generator());

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
        auto messages = gGenerator->generate(message, gFilter);

        size_t length = 0;
        for (auto& message : messages) {
            length += message.data().size();
        }

        printf("RTCM: %4zu bytes | ", length);
        for (auto& message : messages) {
            printf("%4i ", message.id());
        }
        printf("\n");

        for (auto& message : messages) {
            for (auto interface : gInterfaces) {
                interface->write(message.data().data(), message.data().size());
            }
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
        auto message = buffer.str();
        for (auto interface : gInterfaces) {
            interface->write(message.c_str(), message.size());
        }
    } else {
        throw std::runtime_error("Unsupported format");
    }
}

}  // namespace osr_example
