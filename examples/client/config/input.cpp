#include "config.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-dtor"
#pragma GCC diagnostic ignored "-Wnewline-eof"
#pragma GCC diagnostic ignored "-Wmissing-variable-declarations"
#pragma GCC diagnostic ignored "-Winconsistent-missing-destructor-override"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wshadow-field"
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#include <args.hpp>
#pragma GCC diagnostic pop

namespace input {

static args::Group                      gGroup{"Input:"};
static args::ValueFlagList<std::string> gArgs{
    gGroup,
    "input",
    "Add an input interface.\n"
    "Usage: --input <type>:<arguments>\n\n"
    "Arguments:\n"
    "  format=<fmt>[+<fmt>...]\n"
    "  tags=<tag>[+<tag>...]\n"
    "  chain=<stage>[+<stage>...]\n\n"
    "Types and their specific arguments:\n"
    "  stdin:\n"
    "  file:\n"
    "    path=<path>\n"
    "    bps=<bps>\n"
    "  serial:\n"
    "    device=<device>\n"
    "    baudrate=<baudrate>\n"
    "    databits=<5|6|7|8>\n"
    "    stopbits=<1|2>\n"
    "    parity=<none|odd|even>\n"
    "  tcp-client:\n"
    "    host=<host>\n"
    "    port=<port>\n"
    "    reconnect=<bool> (default=true)\n"
    "    path=<path>\n"
    "  tcp-server:\n"
    "    listen=<addr> (default=0.0.0.0)\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "  udp-server:\n"
    "    listen=<addr> (default=0.0.0.0)\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "\n"
    "Stages:\n"
    "  tlf\n"
    "Formats:\n"
    "  all, ubx, nmea, rtcm, ctrl, lpp-uper, lpp-uper-pad\n",
    {"input"},
};

static void setup() {}

static bool parse_bool_option(std::unordered_map<std::string, std::string> const& options,
                              std::string const& type, std::string const& key, bool default_value) {
    if (options.find(key) == options.end()) return default_value;
    auto value = options.at(key);
    if (value == "true") return true;
    if (value == "false") return false;
    throw args::ValidationError("--input " + type + ": `" + key + "` must be a boolean, got `" +
                                value + "'");
}

static InputFormat parse_format(std::string const& str) {
    if (str == "all") return INPUT_FORMAT_ALL;
    if (str == "ubx") return INPUT_FORMAT_UBX;
    if (str == "nmea") return INPUT_FORMAT_NMEA;
    if (str == "rtcm") return INPUT_FORMAT_RTCM;
    if (str == "ctrl") return INPUT_FORMAT_CTRL;
    if (str == "lpp-uper") return INPUT_FORMAT_LPP_UPER;
    if (str == "lpp-uper-pad") return INPUT_FORMAT_LPP_UPER_PAD;
    throw args::ValidationError("--input format: invalid format, got `" + str + "`");
}

static InputFormat parse_format_list(std::string const& str) {
    auto parts  = split(str, '+');
    auto format = INPUT_FORMAT_NONE;
    for (auto const& part : parts) {
        format |= parse_format(part);
    }
    return format;
}

static InputFormat
parse_format_list_from_options(std::unordered_map<std::string, std::string> const& options) {
    if (options.find("format") == options.end()) {
        throw args::ValidationError("--output: missing `format` option");
    }
    return parse_format_list(options.at("format"));
}

static std::string parse_stage(std::string const& str) {
    if (str == "tlf") return "tlf";
    throw args::ValidationError("--output stage: invalid stage, got `" + str + "`");
}

static std::vector<std::string> parse_stages(std::string const& str) {
    auto                     parts = split(str, '+');
    std::vector<std::string> stages;
    for (auto const& part : parts) {
        stages.push_back(parse_stage(part));
    }
    return stages;
}

static std::vector<std::string>
parse_stages_from_options(std::unordered_map<std::string, std::string> const& options) {
    if (options.find("chain") == options.end()) {
        return {};
    } else {
        return parse_stages(options.at("chain"));
    }
}

static std::vector<std::string> parse_tags(std::string const& str) {
    return split(str, '+');
}

static std::vector<std::string>
parse_tags_from_options(std::unordered_map<std::string, std::string> const& options) {
    if (options.find("tags") == options.end()) {
        return {};
    } else {
        return parse_tags(options.at("tags"));
    }
}

static std::unordered_map<std::string, std::string> parse_options(std::string const& str) {
    auto                                         parts = split(str, ',');
    std::unordered_map<std::string, std::string> options;
    for (auto const& part : parts) {
        auto kv = split(part, '=');
        if (kv.size() != 2) {
            throw args::ValidationError("--output: invalid option, got `" + part + "`");
        }
        options[kv[0]] = kv[1];
    }
    return options;
}

static std::unique_ptr<io::Input>
parse_input_stdin(std::unordered_map<std::string, std::string> const& options) {
    return std::make_unique<io::StdinInput>();
}

static std::unique_ptr<io::Input>
parse_input_file(std::unordered_map<std::string, std::string> const& options) {
    if (options.find("path") == options.end()) {
        throw args::ValidationError("--input file: missing `path` option");
    }

    auto path = options.at("path");
    auto bps  = 128 * 10;
    if (options.find("bps") != options.end()) {
        try {
            bps = std::stoi(options.at("bps"));
        } catch (...) {
            throw args::ParseError("--input file: `bps` must be an integer, got `" +
                                   options.at("bps") + "'");
        }
    }

    auto tick_interval  = std::chrono::milliseconds(100);
    auto bytes_per_tick = static_cast<size_t>((bps + 9) / 10);
    return std::unique_ptr<io::Input>(new io::FileInput(path, bytes_per_tick, tick_interval));
}

static io::BaudRate parse_baudrate(std::string const& str) {
    long baud_rate = 0;
    try {
        baud_rate = std::stol(str);
    } catch (...) {
        throw args::ParseError("--input serial: `baudrate` must be an integer, got `" + str + "'");
    }

    if (baud_rate == 50) return io::BaudRate::BR50;
    if (baud_rate == 75) return io::BaudRate::BR75;
    if (baud_rate == 110) return io::BaudRate::BR110;
    if (baud_rate == 134) return io::BaudRate::BR134;
    if (baud_rate == 150) return io::BaudRate::BR150;
    if (baud_rate == 200) return io::BaudRate::BR200;
    if (baud_rate == 300) return io::BaudRate::BR300;
    if (baud_rate == 600) return io::BaudRate::BR600;
    if (baud_rate == 1200) return io::BaudRate::BR1200;
    if (baud_rate == 1800) return io::BaudRate::BR1800;
    if (baud_rate == 2400) return io::BaudRate::BR2400;
    if (baud_rate == 4800) return io::BaudRate::BR4800;
    if (baud_rate == 9600) return io::BaudRate::BR9600;
    if (baud_rate == 19200) return io::BaudRate::BR19200;
    if (baud_rate == 38400) return io::BaudRate::BR38400;
    if (baud_rate == 57600) return io::BaudRate::BR57600;
    if (baud_rate == 115200) return io::BaudRate::BR115200;
    if (baud_rate == 230400) return io::BaudRate::BR230400;
    if (baud_rate == 460800) return io::BaudRate::BR460800;
    if (baud_rate == 500000) return io::BaudRate::BR500000;
    if (baud_rate == 576000) return io::BaudRate::BR576000;
    if (baud_rate == 921600) return io::BaudRate::BR921600;
    if (baud_rate == 1000000) return io::BaudRate::BR1000000;
    if (baud_rate == 1152000) return io::BaudRate::BR1152000;
    if (baud_rate == 1500000) return io::BaudRate::BR1500000;
    if (baud_rate == 2000000) return io::BaudRate::BR2000000;
    if (baud_rate == 2500000) return io::BaudRate::BR2500000;
    if (baud_rate == 3000000) return io::BaudRate::BR3000000;
    if (baud_rate == 3500000) return io::BaudRate::BR3500000;
    if (baud_rate == 4000000) return io::BaudRate::BR4000000;
    throw args::ParseError("--input serial: `baudrate` must be a valid baud rate, got `" + str +
                           "'");
}

static io::DataBits parse_databits(std::string const& str) {
    long databits = 0;
    try {
        databits = std::stol(str);
    } catch (...) {
        throw args::ParseError("--input serial: `data` must be an integer, got `" + str + "'");
    }

    if (databits == 5) return io::DataBits::FIVE;
    if (databits == 6) return io::DataBits::SIX;
    if (databits == 7) return io::DataBits::SEVEN;
    if (databits == 8) return io::DataBits::EIGHT;
    throw args::ParseError("--input serial: `data` must be 5, 6, 7, or 8, got `" + str + "'");
}

static io::StopBits parse_stopbits(std::string const& str) {
    long stopbits = 0;
    try {
        stopbits = std::stol(str);
    } catch (...) {
        throw args::ParseError("--input serial: `stop` must be an integer, got `" + str + "'");
    }

    if (stopbits == 1) return io::StopBits::ONE;
    if (stopbits == 2) return io::StopBits::TWO;
    throw args::ParseError("--input serial: `stop` must be 1 or 2, got `" + str + "'");
}

static io::ParityBit parse_paritybit(std::string const& str) {
    if (str == "none") return io::ParityBit::NONE;
    if (str == "odd") return io::ParityBit::ODD;
    if (str == "even") return io::ParityBit::EVEN;
    throw args::ParseError("--input serial: `parity` must be none, odd, or even, got `" + str +
                           "'");
}

static std::unique_ptr<io::Input>
parse_serial(std::unordered_map<std::string, std::string> const& options) {
    if (options.find("device") == options.end()) {
        throw args::RequiredError("--input serial: missing `device` option");
    }

    auto baud_rate  = io::BaudRate::BR115200;
    auto data_bits  = io::DataBits::EIGHT;
    auto stop_bits  = io::StopBits::ONE;
    auto parity_bit = io::ParityBit::NONE;

    auto baud_rate_it = options.find("baudrate");
    if (baud_rate_it != options.end()) baud_rate = parse_baudrate(baud_rate_it->second);

    auto data_bits_it = options.find("data");
    if (data_bits_it != options.end()) data_bits = parse_databits(data_bits_it->second);

    auto stop_bits_it = options.find("stop");
    if (stop_bits_it != options.end()) stop_bits = parse_stopbits(stop_bits_it->second);

    auto parity_bit_it = options.find("parity");
    if (parity_bit_it != options.end()) parity_bit = parse_paritybit(parity_bit_it->second);

    auto device = options.at("device");
    return std::unique_ptr<io::Input>(
        new io::SerialInput(device, baud_rate, data_bits, stop_bits, parity_bit));
}

static std::unique_ptr<io::Input>
parse_tcp_client(std::unordered_map<std::string, std::string> const& options) {
    auto reconnect = true;
    if (options.find("reconnect") != options.end()) {
        if (options.at("reconnect") == "true") {
            reconnect = true;
        } else if (options.at("reconnect") == "false") {
            reconnect = false;
        } else {
            throw args::ValidationError("--input tcp-client: `reconnect` must be a boolean, got `" +
                                        options.at("reconnect") + "'");
        }
    }

    if (options.find("host") != options.end()) {
        if (options.find("host") == options.end()) {
            throw args::RequiredError("--input tcp-client: missing `host` option");
        } else if (options.find("port") == options.end()) {
            throw args::RequiredError("--input tcp-client: missing `port` option");
        } else if (options.find("path") != options.end()) {
            throw args::RequiredError(
                "--input tcp-client: `path` cannot be used with `host` and `port`");
        }

        auto host = options.at("host");
        auto port = 0;
        try {
            port = std::stoi(options.at("port"));
        } catch (...) {
            throw args::ParseError("--input tcp-client: `port` must be an integer, got `" +
                                   options.at("port") + "'");
        }

        if (port < 0 || port > 65535) {
            throw args::ParseError(
                "--input tcp-client: `port` must be in the range [0, 65535], got `" +
                std::to_string(port) + "'");
        }

        return std::unique_ptr<io::Input>(
            new io::TcpClientInput(host, static_cast<uint16_t>(port), reconnect));
    } else if (options.find("path") != options.end()) {
        auto path = options.at("path");
        return std::unique_ptr<io::Input>(new io::TcpClientInput(path, reconnect));
    } else {
        throw args::RequiredError("--input tcp-client: missing `host` and `port` or `path` option");
    }
}

static std::unique_ptr<io::Input>
parse_tcp_server(std::unordered_map<std::string, std::string> const& options) {
    if (options.find("listen") != options.end() || options.find("port") != options.end()) {
        if (options.find("port") == options.end()) {
            throw args::RequiredError("--input tcp-server: missing `port` option");
        } else if (options.find("path") != options.end()) {
            throw args::RequiredError(
                "--input tcp-server: `path` cannot be used with `listen` and `port`");
        }

        std::string listen{};
        if (options.find("listen") == options.end()) {
            listen = "0.0.0.0";
        } else {
            listen = options.at("listen");
        }

        auto port = 0;
        try {
            port = std::stoi(options.at("port"));
        } catch (...) {
            throw args::ParseError("--input tcp-server: `port` must be an integer, got `" +
                                   options.at("port") + "'");
        }

        if (port < 0 || port > 65535) {
            throw args::ParseError(
                "--input tcp-server: `port` must be in the range [0, 65535], got `" +
                std::to_string(port) + "'");
        }

        return std::unique_ptr<io::Input>(
            new io::TcpServerInput(listen, static_cast<uint16_t>(port)));
    } else if (options.find("path") != options.end()) {
        auto path = options.at("path");
        return std::unique_ptr<io::Input>(new io::TcpServerInput(path));
    } else {
        throw args::RequiredError(
            "--input tcp-server: missing `listen` and `port` or `path` option");
    }
}

static std::unique_ptr<io::Input>
parse_udp_server(std::unordered_map<std::string, std::string> const& options) {
    if (options.find("listen") != options.end() || options.find("port") != options.end()) {
        if (options.find("port") == options.end()) {
            throw args::RequiredError("--input udp-server: missing `port` option");
        } else if (options.find("path") != options.end()) {
            throw args::RequiredError(
                "--input udp-server: `path` cannot be used with `listen` and `port`");
        }

        std::string listen{};
        if (options.find("listen") == options.end()) {
            listen = "0.0.0.0";
        } else {
            listen = options.at("listen");
        }

        auto port = 0;
        try {
            port = std::stoi(options.at("port"));
        } catch (...) {
            throw args::ParseError("--input udp-server: `port` must be an integer, got `" +
                                   options.at("port") + "'");
        }

        if (port < 0 || port > 65535) {
            throw args::ParseError(
                "--input udp-server: `port` must be in the range [0, 65535], got `" +
                std::to_string(port) + "'");
        }

        return std::unique_ptr<io::Input>(
            new io::UdpServerInput(listen, static_cast<uint16_t>(port)));
    } else if (options.find("path") != options.end()) {
        auto path = options.at("path");
        return std::unique_ptr<io::Input>(new io::UdpServerInput(path));
    } else {
        throw args::RequiredError(
            "--input udp-server: missing `listen` and `port` or `path` option");
    }
}

static InputInterface parse_interface(std::string const& source) {
    std::unordered_map<std::string, std::string> options;

    auto parts = split(source, ':');
    if (parts.size() == 1) {
        // No options
    } else if (parts.size() == 2) {
        options = parse_options(parts[1]);
    } else {
        throw args::ValidationError("--input: invalid input, got `" + source + "`");
    }

    auto format = parse_format_list_from_options(options);
    auto tags   = parse_tags_from_options(options);
    auto stages = parse_stages_from_options(options);
    auto print  = parse_bool_option(options, parts[0], "print", false);

    std::unique_ptr<io::Input> input;
    if (parts[0] == "stdin") input = parse_input_stdin(options);
    if (parts[0] == "file") input = parse_input_file(options);
    if (parts[0] == "serial") input = parse_serial(options);
    if (parts[0] == "tcp-client") input = parse_tcp_client(options);
    if (parts[0] == "tcp-server") input = parse_tcp_server(options);
    if (parts[0] == "udp-server") input = parse_udp_server(options);

    if (input) {
        return {format, print, std::move(input), tags, stages};
    }

    throw args::ValidationError("--input: invalid input type, got `" + parts[0] + "`");
}

static void parse(Config* config) {
    for (auto const& input : gArgs.Get()) {
        config->input.inputs.push_back(parse_interface(input));
    }

    for (auto& input : config->input.inputs) {
        for (auto& tag : input.tags) {
            config->register_tag(tag);
        }
    }
}

static char const* input_type(io::Input* input) {
    if (dynamic_cast<io::StdinInput*>(input)) return "stdin";
    if (dynamic_cast<io::FileInput*>(input)) return "file";
    if (dynamic_cast<io::SerialInput*>(input)) return "serial";
    if (dynamic_cast<io::TcpClientInput*>(input)) return "tcp-client";
    if (dynamic_cast<io::TcpServerInput*>(input)) return "tcp-server";
    if (dynamic_cast<io::UdpServerInput*>(input)) return "udp-server";
    return "unknown";
}

static void dump(InputConfig const& config) {
    for (auto const& input : config.inputs) {
        DEBUGF("%p: %s", input.interface.get(), input_type(input.interface.get()));
        DEBUG_INDENT_SCOPE();
        DEBUGF("format: %s%s%s%s%s%s", (input.format & INPUT_FORMAT_UBX) ? "UBX " : "",
               (input.format & INPUT_FORMAT_NMEA) ? "NMEA " : "",
               (input.format & INPUT_FORMAT_RTCM) ? "RTCM " : "",
               (input.format & INPUT_FORMAT_CTRL) ? "CTRL " : "",
               (input.format & INPUT_FORMAT_LPP_UPER) ? "LPP-UPER " : "",
               (input.format & INPUT_FORMAT_LPP_UPER_PAD) ? "LPP-UPER-PAD " : "");

        std::stringstream tag_ss;
        for (auto const& tag : input.tags) {
            tag_ss << tag << " ";
        }
        auto tag_str = tag_ss.str();
        DEBUGF("tags: %s", tag_str.c_str());

        DEBUGF("print: %s", input.print ? "true" : "false");
        auto stdin_input = dynamic_cast<io::StdinInput*>(input.interface.get());
        if (stdin_input) continue;

        auto file_input = dynamic_cast<io::FileInput*>(input.interface.get());
        if (file_input) {
            DEBUGF("path: \"%s\"", file_input->path().c_str());
            continue;
        }

        auto serial_input = dynamic_cast<io::SerialInput*>(input.interface.get());
        if (serial_input) {
            DEBUGF("device: \"%s\"", serial_input->device().c_str());
            DEBUGF("baudrate: %s", io::baud_rate_to_str(serial_input->baud_rate()));
            DEBUGF("data: %s", io::data_bits_to_str(serial_input->data_bits()));
            DEBUGF("stop: %s", io::stop_bits_to_str(serial_input->stop_bits()));
            DEBUGF("parity: %s", io::parity_bit_to_str(serial_input->parity_bit()));
            continue;
        }

        auto tcp_client_input = dynamic_cast<io::TcpClientInput*>(input.interface.get());
        if (tcp_client_input) {
            if (tcp_client_input->path().empty()) {
                DEBUGF("host: \"%s\"", tcp_client_input->host().c_str());
                DEBUGF("port: %u", tcp_client_input->port());
            } else {
                DEBUGF("path: \"%s\"", tcp_client_input->path().c_str());
            }
            DEBUGF("reconnect: %s", tcp_client_input->reconnect() ? "true" : "false");
            continue;
        }

        auto tcp_server_input = dynamic_cast<io::TcpServerInput*>(input.interface.get());
        if (tcp_server_input) {
            if (tcp_server_input->path().empty()) {
                DEBUGF("listen: \"%s\"", tcp_server_input->listen().c_str());
                DEBUGF("port: %u", tcp_server_input->port());
            } else {
                DEBUGF("path: \"%s\"", tcp_server_input->path().c_str());
            }
            continue;
        }

        auto udp_server_input = dynamic_cast<io::UdpServerInput*>(input.interface.get());
        if (udp_server_input) {
            if (udp_server_input->path().empty()) {
                DEBUGF("listen: \"%s\"", udp_server_input->listen().c_str());
                DEBUGF("port: %u", udp_server_input->port());
            } else {
                DEBUGF("path: \"%s\"", udp_server_input->path().c_str());
            }
            continue;
        }

        DEBUGF("unimplemented type");
    }
}

}  // namespace input
