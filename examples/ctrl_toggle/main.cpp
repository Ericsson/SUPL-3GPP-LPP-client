#include <cstring>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <version.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hpp>
EXTERNAL_WARNINGS_POP

#include <loglet/loglet.hpp>

LOGLET_MODULE(ctrl_toggle);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(ctrl_toggle)

struct Cell {
    int                mcc;
    int                mnc;
    int                tac;
    unsigned long long cid;
    bool               is_nr;
};

struct Config {
    bool               has_imsi;
    unsigned long long imsi;

    int update_ms;

    std::vector<Cell> cells;
};

[[noreturn]] static void loop(Config& config) {
    // Open TCP server
    auto socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        ERRORF("socket failed: %s", strerror(errno));
        exit(1);
    }

    // Bind to port
    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(13226);
    addr.sin_addr.s_addr = INADDR_ANY;

    // Set reuse address
    int reuse = 1;
    if (::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        ERRORF("setsockopt failed: %s", strerror(errno));
        exit(1);
    }

    auto socket_addr = reinterpret_cast<struct sockaddr*>(&addr);
    auto socket_size = static_cast<socklen_t>(sizeof(addr));
    if (::bind(socket, socket_addr, socket_size) < 0) {
        ERRORF("bind failed: %s", strerror(errno));
        exit(1);
    }

    // Listen for connections
    if (::listen(socket, 5) < 0) {
        ERRORF("listen failed: %s", strerror(errno));
        exit(1);
    }

    INFOF("listening on port 13226");

    // Accept connections
    for (;;) {
        auto client = ::accept(socket, nullptr, nullptr);
        if (client < 0) {
            ERRORF("accept failed: %s", strerror(errno));
            exit(1);
        }

        DEBUGF("client connected");

        if (config.has_imsi) {
            char buffer[1024];
            auto length = snprintf(buffer, sizeof(buffer), "/IDENTITY,IMSI,%llu\r\n", config.imsi);
            INFOF("command: %.*s", length - 2, buffer);
            auto result = ::send(client, buffer, static_cast<size_t>(length), MSG_NOSIGNAL);
            if (result < 0) {
                ERRORF("send failed: %s", strerror(errno));
                exit(1);
            }

            usleep(1000 * 1000);
        }

        auto connected = true;
        while (connected) {
            for (auto& cell : config.cells) {
                char buffer[1024];
                auto length =
                    snprintf(buffer, sizeof(buffer), "/CID,%s,%d,%d,%d,%llu\r\n",
                             cell.is_nr ? "N" : "L", cell.mcc, cell.mnc, cell.tac, cell.cid);
                INFOF("command: %.*s", length - 2, buffer);
                auto result = ::send(client, buffer, static_cast<size_t>(length), MSG_NOSIGNAL);
                if (result < 0) {
                    ERRORF("send failed: %s", strerror(errno));
                    connected = false;
                    break;
                }

                auto ms = 1000 * config.update_ms;
                usleep(static_cast<__useconds_t>(ms));
            }
        }

        // Close client
        DEBUGF("client disconnected");
        ::close(client);
    }
}

static std::vector<std::string> split(std::string const& str, char delimiter) {
    std::vector<std::string> result;
    std::string              token;
    std::istringstream       token_stream(str);
    while (std::getline(token_stream, token, delimiter)) {
        result.push_back(token);
    }
    return result;
}

static bool parse_cell(std::string const& cell, Cell& result) {
    auto parts = split(cell, ':');
    if (parts.size() != 5) {
        ERRORF("invalid cell: %s", cell.c_str());
        return false;
    }

    if (parts[0] == "LTE") {
        result.is_nr = false;
    } else if (parts[0] == "NR") {
        result.is_nr = true;
    } else {
        ERRORF("invalid cell type: %s", parts[0].c_str());
        return false;
    }

    result.mcc = std::stoi(parts[1]);
    result.mnc = std::stoi(parts[2]);
    result.tac = std::stoi(parts[3]);
    result.cid = std::stoull(parts[4]);
    return true;
}

int main(int argc, char** argv) {
    args::ArgumentParser parser("Control Toggle (" CLIENT_VERSION ")");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    args::Group arguments{"Arguments:"};

    args::ValueFlag<unsigned long long> imsi_arg{
        arguments, "imsi", "First send IMSI identity", {"imsi"}};

    args::ValueFlag<int> ms_between_cells{
        arguments, "X", "Update cells every X seconds", {"update-rate"}};
    ms_between_cells.HelpDefault("5");

    args::PositionalList<std::string> cell_args{
        arguments, "cells", "List of cells: [LTE|NR]:mcc:mnc:tac:cid\ne.g. LTE:240:1:1:1"};

    parser.Add(arguments);

    try {
        parser.ParseCLI(argc, argv);
        if (version) {
            std::cout << "Control Toggle " << CLIENT_VERSION << std::endl;
            std::cout << "  Commit: " << GIT_COMMIT_HASH << (GIT_DIRTY ? "-dirty" : "") << " ("
                      << GIT_BRANCH << ")" << std::endl;
            std::cout << "  Built: " << BUILD_DATE << " [" << BUILD_TYPE << "]" << std::endl;
            std::cout << "  Compiler: " << BUILD_COMPILER << std::endl;
            std::cout << "  Platform: " << BUILD_SYSTEM << " (" << BUILD_ARCH << ")" << std::endl;
            return 1;
        }

        INFOF("control toggle");

        Config config{};
        config.has_imsi  = false;
        config.update_ms = 5 * 1000;
        if (imsi_arg) {
            config.has_imsi = true;
            config.imsi     = imsi_arg.Get();
        }

        if (ms_between_cells) {
            config.update_ms = ms_between_cells.Get() * 1000;
        }

        for (auto const& cell : cell_args.Get()) {
            Cell result{};
            if (!parse_cell(cell, result)) {
                return 1;
            }

            config.cells.push_back(result);
        }

        if (!config.has_imsi && config.cells.empty()) {
            ERRORF("no cells or IMSI provided");
            return 1;
        }

        loop(config);
    } catch (args::ValidationError const& e) {
        parser.Help(std::cerr);
        ERRORF("validation error: %s", e.what());
        return 1;
    } catch (args::Help const&) {
        parser.Help(std::cerr);
        return 1;
    } catch (args::ParseError const& e) {
        parser.Help(std::cerr);
        ERRORF("parse error: %s", e.what());
        return 1;
    } catch (std::exception const& e) {
        ERRORF("unknown error: %s", e.what());
        return 1;
    }

#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    return 1;
#endif
}
