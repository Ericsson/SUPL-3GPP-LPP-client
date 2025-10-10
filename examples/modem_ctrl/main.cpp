#include "options.hpp"

#include <inttypes.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <modem/modem.hpp>
#include <scheduler/scheduler.hpp>
#include <loglet/loglet.hpp>

LOGLET_MODULE(modem_ctrl);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(modem_ctrl)

static int start_server(uint16_t port) {
    FUNCTION_SCOPE();
    DEBUGF("starting server on port %u", port);
    
    auto socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        ERRORF("failed to create socket: %s", strerror(errno));
        exit(1);
    }
    VERBOSEF("created socket fd=%d", socket);

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    auto socket_addr = reinterpret_cast<struct sockaddr*>(&addr);
    auto socket_size = static_cast<socklen_t>(sizeof(addr));
    if (::bind(socket, socket_addr, socket_size) < 0) {
        ERRORF("failed to bind socket: %s", strerror(errno));
        exit(1);
    }
    VERBOSEF("bound socket to port %u", port);

    int opt = 1;
    if (::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        ERRORF("failed to set socket options: %s", strerror(errno));
        exit(1);
    }
    VERBOSEF("set SO_REUSEADDR on socket");

    if (::listen(socket, 5) < 0) {
        ERRORF("failed to listen on socket: %s", strerror(errno));
        exit(1);
    }

    INFOF("listening on port %u", port);
    return socket;
}

[[noreturn]] static void loop(Config& config) {
    FUNCTION_SCOPE();
    INFOF("modem-ctrl starting");

    modem::Modem device{std::move(config.input), std::move(config.output)};
    DEBUGF("modem device created");

    scheduler::Scheduler scheduler{};
    (void)device.schedule(scheduler);
    VERBOSEF("modem scheduled");

    device.enable_echo();
    DEBUGF("echo enabled");

    modem::SupportedCregModes creg_modes{};
    if (!device.list_creg(scheduler, creg_modes)) {
        ERRORF("failed to get CREG modes");
    } else {
        DEBUGF("CREG modes: mode_1=%d mode_2=%d", creg_modes.mode_1, creg_modes.mode_2);
        if (creg_modes.mode_2) {
            device.set_creg(2);
            INFOF("set CREG mode to 2");
        } else if (creg_modes.mode_1) {
            device.set_creg(1);
            INFOF("set CREG mode to 1");
        } else {
            ERRORF("no supported CREG modes");
        }
    }

    device.set_cops(2);
    DEBUGF("set COPS mode to 2");

    auto socket       = start_server(config.port);
    auto send_command = [&](int client, char const* command) {
        FUNCTION_SCOPE();
        DEBUGF("sending command to client: %s", command);
        char buffer[1024];
        auto length = snprintf(buffer, sizeof(buffer), "%s\r\n", command);
        auto result = ::send(client, buffer, static_cast<size_t>(length), MSG_NOSIGNAL);
        if (result < 0) {
            ERRORF("failed to send command: %s", strerror(errno));
            return false;
        }
        VERBOSEF("sent %ld bytes to client", result);
        return true;
    };

    for (;;) {
        VERBOSEF("waiting for client connection");
        auto client = ::accept(socket, nullptr, nullptr);
        if (client < 0) {
            ERRORF("accept failed: %s", strerror(errno));
            continue;
        }
        INFOF("client connected (fd=%d)", client);

        modem::Cimi cimi{};
        if (!device.get_cimi(scheduler, cimi)) {
            ERRORF("failed to get CIMI");
            ::close(client);
            continue;
        }
        DEBUGF("CIMI: %" PRIu64, cimi.imsi);

        char send_buffer[1024];
        snprintf(send_buffer, sizeof(send_buffer), "/IDENTITY,IMSI,%" PRIu64, cimi.imsi);
        if (!send_command(client, send_buffer)) {
            ::close(client);
            continue;
        }

        auto connected = true;
        while (connected) {
            VERBOSEF("querying modem status");
            modem::Cops cops{};
            modem::Creg reg{};

            auto cops_result = device.get_cops(scheduler, cops);
            auto reg_result  = device.get_creg(scheduler, reg);
            if (!cops_result || !reg_result) {
                ERRORF("failed to get COPS or CREG");
            } else {
                if (cops.format == 2) {
                    auto act_type = (cops.act >= 0 && cops.act < 7) ? "GSM" :
                                   (cops.act >= 7 && cops.act < 11) ? "LTE" : "UNKNOWN";
                    INFOF("operator: %d:%d %s", cops.mcc, cops.mnc, act_type);
                    VERBOSEF("COPS: format=%d mcc=%d mnc=%d act=%d", cops.format, cops.mcc, cops.mnc, cops.act);
                }
                if (reg.mode != 0) {
                    auto act_type = (reg.act >= 0 && reg.act < 7) ? "GSM" :
                                   (reg.act >= 7 && reg.act < 11) ? "LTE" : "UNKNOWN";
                    INFOF("cell: %u:%u %s", reg.lac, reg.ci, act_type);
                    VERBOSEF("CREG: mode=%d lac=%u ci=%u act=%d", reg.mode, reg.lac, reg.ci, reg.act);
                }
                if (reg.mode != 0 && cops.format == 2) {
                    if (reg.is_gsm()) {
                        snprintf(send_buffer, sizeof(send_buffer), "/CID,G,%d,%d,%u,%u", cops.mcc,
                                 cops.mnc, reg.lac, reg.ci);
                        DEBUGF("sending GSM cell info");
                    } else {
                        snprintf(send_buffer, sizeof(send_buffer), "/CID,L,%d,%d,%u,%u", cops.mcc,
                                 cops.mnc, reg.lac, reg.ci);
                        DEBUGF("sending LTE cell info");
                    }
                    if (!send_command(client, send_buffer)) {
                        ::close(client);
                        continue;
                    }
                }
            }

            unsigned int sleep_seconds = 1;
            if (config.update_interval > 1) {
                sleep_seconds = static_cast<unsigned int>(config.update_interval);
            }
            VERBOSEF("sleeping for %u seconds", sleep_seconds);
            sleep(sleep_seconds);
        }

        INFOF("client disconnected");
        ::close(client);
    }
}

int main(int argc, char** argv) {
    auto config = parse_configuration(argc, argv);
    
    loglet::set_level(config.logging.log_level);
    loglet::set_color_enable(config.logging.color);
    loglet::set_always_flush(config.logging.flush);
    
    INFOF("log level: %d", static_cast<int>(config.logging.log_level));
    DEBUGF("configuration parsed successfully");
    
    loop(config);
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    return 0;
#endif
}
