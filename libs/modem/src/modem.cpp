#include <interface/interface.hpp>
#include <modem/modem.hpp>

#include <cstring>

#if DEBUG_MODEM == 1
#define MODEM_PRINT(...) printf(__VA_ARGS__)
#else
#define MODEM_PRINT(...)
#endif

#define MODEM_PRINT_ERROR(...) fprintf(stderr, __VA_ARGS__)

namespace modem {

Device::Device(std::unique_ptr<interface::Interface> interface) MODEM_NOEXCEPT
    : mInterface(std::move(interface)) {}

void Device::disable_echo() {
    send_requst("ATE0");
    auto response = wait_for_response();
    if (!response.success) {
        MODEM_PRINT_ERROR("modem: failed to disable echo\n");
        return;
    }
}

void Device::enable_echo() {
    send_requst("ATE1");
    auto response = wait_for_response();
    if (!response.success) {
        MODEM_PRINT_ERROR("modem: failed to enable echo\n");
        return;
    }
}

void Device::get_cgmi() {
    send_requst("AT+CGMI");
    auto response = wait_for_response();
    if (!response.success) {
        MODEM_PRINT_ERROR("modem: failed to get manufacturer\n");
        return;
    }
}

Cimi Device::get_cimi() {
    send_requst("AT+CIMI");
    auto response = wait_for_response();
    if (!response.success) {
        MODEM_PRINT_ERROR("modem: failed to get IMSI\n");
        return {};
    }

    Cimi cimi{};
    for (auto const& line : response.lines) {
        if (line.size() > 0) {
            cimi.imsi = std::stoull(line);
        }
    }

    return cimi;
}

Creg Device::get_creg() {
    send_requst("AT+CREG?");
    auto response = wait_for_response();
    if (!response.success) {
        MODEM_PRINT_ERROR("modem: failed to get registration status\n");
        return {};
    } else {
        Creg creg{};
        for (auto const& line : response.lines) {
            if (line.find("+CREG:") != std::string::npos) {
                sscanf(line.c_str(), "+CREG: %d,%d,\"%x\",\"%x\",%d", &creg.mode, &creg.status,
                       &creg.lac, &creg.ci, &creg.act);
            }
        }
        return creg;
    }
}

void Device::set_creg(int mode) {
    char buffer[1024];
    auto length = snprintf(buffer, sizeof(buffer), "AT+CREG=%d", mode);
    send_requst(buffer);
    auto response = wait_for_response();
    if (!response.success) {
        MODEM_PRINT_ERROR("modem: failed to set registration status\n");
        return;
    }
}

SupportedCregModes Device::list_creg() {
    send_requst("AT+CREG=?");
    auto response = wait_for_response();
    if (!response.success) {
        MODEM_PRINT_ERROR("modem: failed to list registration status\n");
        return {};
    } else {
        SupportedCregModes modes{};
        for (auto const& line : response.lines) {
            if (line.find("+CREG: (0)") != std::string::npos) {
                modes.mode_0 = true;
            } else if (line.find("+CREG: (0-1)") != std::string::npos) {
                modes.mode_0 = true;
                modes.mode_1 = true;
            } else if (line.find("+CREG: (0-2)") != std::string::npos) {
                modes.mode_0 = true;
                modes.mode_1 = true;
                modes.mode_2 = true;
            } else {
                MODEM_PRINT("Unknown CREG mode: '%s'\n", line.c_str());
            }
        }
        return modes;
    }
}

Cops Device::get_cops() {
    send_requst("AT+COPS?");
    auto response = wait_for_response();
    if (!response.success) {
        MODEM_PRINT_ERROR("modem: failed to get operator\n");
        return {};
    }

    Cops cops{};
    for (auto const& line : response.lines) {
        if (line.find("+COPS:") != std::string::npos) {
            if (sscanf(line.c_str(), "+COPS: %d,%d", &cops.mode, &cops.format) == 2) {
                if (cops.mode == 0 && cops.format == 2) {
                    sscanf(line.c_str(), "+COPS: %d,%d,\"%03d%d\",%d", &cops.mode, &cops.format,
                           &cops.mcc, &cops.mnc, &cops.act);
                }
            }
        }
    }

    return cops;
}

void Device::set_cops_format(int format) {
    char buffer[1024];
    auto length = snprintf(buffer, sizeof(buffer), "AT+COPS=3,%d,", format);
    send_requst(buffer);
    auto response = wait_for_response();
    if (!response.success) {
        MODEM_PRINT_ERROR("modem: failed to set operator format\n");
        return;
    }
}

SupportedCopsModes Device::list_cops() {
    send_requst("AT+COPS=?");
    auto response = wait_for_response();
    if (!response.success) {
        MODEM_PRINT_ERROR("modem: failed to list operators\n");
        return {};
    } else {
        SupportedCopsModes modes{};
        return modes;
    }
}

void Device::send_requst(char const* request) {
    MODEM_PRINT("SEND: %s\n", request);
    char buffer[1024];
    auto length = snprintf(buffer, sizeof(buffer), "%s\r\n", request);
    mInterface->write(buffer, length);
}

Response Device::wait_for_response() {
    Response response{};
    for (;;) {
        if (!mInterface->is_open()) {
            response.success = false;
            return response;
        }

        MODEM_PRINT("RECV: ");

        std::string line;
        line.reserve(256);
        for (;;) {
            char ch;
            auto length = mInterface->read(&ch, 1);
            if (length != 1) {
                response.success = false;
                return response;
            }

            if (ch == '\n') {
                MODEM_PRINT("<L>");
            } else if (ch == '\r') {
                MODEM_PRINT("<R>");
            } else {
                MODEM_PRINT("%c", ch);
            }

            line.push_back(ch);
            if (line.size() >= 2 && line[line.size() - 2] == '\r' &&
                line[line.size() - 1] == '\n') {
                line.resize(line.size() - 2);
                break;
            }
        }

        MODEM_PRINT("\n");
        if (line == "OK") {
            response.success = true;
            return response;
        } else if (line == "ERROR") {
            response.success = false;
            return response;
        } else if (line.size() > 0) {
            response.lines.push_back(line);
        }
    }
}

}  // namespace modem
