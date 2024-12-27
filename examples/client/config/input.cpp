
namespace input {

static args::Group                      gGroup{"Input:"};
static args::ValueFlagList<std::string> gArgs{
    gGroup,
    "input",
    "Add an input interface.\n"
    "Usage: --input <type>:<arguments>\n\n"
    "Arguments:\n"
    "  format=<fmt>[+<fmt>...]\n\n"
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
    "    reconnect=<true|false>\n"
    "  tcp-server:\n"
    "    port=<port>\n"
    "  udp-client:\n"
    "    host=<host>\n"
    "    port=<port>\n"
    "  udp-server:\n"
    "    port=<port>\n"
    "  unix-socket:\n"
    "    path=<path>\n"
    "  i2c:\n"
    "    device=<device>\n"
    "    address=<address>\n"
    "\n"
    "Formats:\n"
    "  all, ubx, nmea, rtcm, ctrl, lpp-uper\n",
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
    if (str == "lpp-uper") return INPUT_FORMAT_LPP;
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

static InputInterface
parse_input_stdin(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "stdin", "print", false);
    auto input  = std::unique_ptr<io::Input>(new io::StdinInput());
    return {format, print, std::move(input)};
}

static InputInterface
parse_input_file(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "input", "print", false);
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

    auto input = std::unique_ptr<io::Input>(new io::FileInput(path, bytes_per_tick, tick_interval));
    return {format, print, std::move(input)};
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

static InputInterface parse_serial(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "serial", "print", false);
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
    auto input  = std::unique_ptr<io::Input>(
        new io::SerialInput(device, baud_rate, data_bits, stop_bits, parity_bit));
    return {format, print, std::move(input)};
}

static InputInterface
parse_tcp_client(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "tcp-client", "print", false);
    if (options.find("host") == options.end()) {
        throw args::RequiredError("--input tcp-client: missing `host` option");
    }
    if (options.find("port") == options.end()) {
        throw args::RequiredError("--input tcp-client: missing `port` option");
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
        throw args::ParseError("--input tcp-client: `port` must be in the range [0, 65535], got `" +
                               std::to_string(port) + "'");
    }

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

    // TODO(ewasjon): Implement reconnect

    auto input = std::unique_ptr<io::Input>(new io::TcpClientInput(host, port));
    return {format, print, std::move(input)};
}

static InputInterface
parse_tcp_server(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "tcp-server", "print", false);
    if (options.find("port") == options.end()) {
        throw args::RequiredError("--input tcp-server: missing `port` option");
    }

    auto port = 0;
    try {
        port = std::stoi(options.at("port"));
    } catch (...) {
        throw args::ParseError("--input tcp-server: `port` must be an integer, got `" +
                               options.at("port") + "'");
    }

    if (port < 0 || port > 65535) {
        throw args::ParseError("--input tcp-server: `port` must be in the range [0, 65535], got `" +
                               std::to_string(port) + "'");
    }

    auto input = std::unique_ptr<io::Input>(new io::TcpServerInput("0.0.0.0", port));
    return {format, print, std::move(input)};
}

static InputInterface
parse_udp_client(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "udp-client", "print", false);
    if (options.find("host") == options.end()) {
        throw args::RequiredError("--input udp-client: missing `host` option");
    }
    if (options.find("port") == options.end()) {
        throw args::RequiredError("--input udp-client: missing `port` option");
    }

    auto host = options.at("host");
    auto port = 0;
    try {
        port = std::stoi(options.at("port"));
    } catch (...) {
        throw args::ParseError("--input udp-client: `port` must be an integer, got `" +
                               options.at("port") + "'");
    }

    if (port < 0 || port > 65535) {
        throw args::ParseError("--input udp-client: `port` must be in the range [0, 65535], got `" +
                               std::to_string(port) + "'");
    }

    // TODO(ewasjon): Implement
    throw args::RequiredError("Not implemented");
    return {};
}

static InputInterface
parse_udp_server(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "udp-server", "print", false);
    if (options.find("port") == options.end()) {
        throw args::RequiredError("--input udp-server: missing `port` option");
    }

    auto port = 0;
    try {
        port = std::stoi(options.at("port"));
    } catch (...) {
        throw args::ParseError("--input udp-server: `port` must be an integer, got `" +
                               options.at("port") + "'");
    }

    if (port < 0 || port > 65535) {
        throw args::ParseError("--input udp-server: `port` must be in the range [0, 65535], got `" +
                               std::to_string(port) + "'");
    }

    auto input = std::unique_ptr<io::Input>(new io::UdpServerInput("0.0.0.0", port));
    return {format, print, std::move(input)};
}

static InputInterface
parse_unix_socket(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "unix-socket", "print", false);
    if (options.find("path") == options.end()) {
        throw args::RequiredError("--input unix-socket: missing `path` option");
    }

    auto path = options.at("path");

    // TODO(ewasjon): Implement
    throw args::RequiredError("Not implemented");
    return {};
}

static InputInterface parse_i2c(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "i2c", "print", false);
    if (options.find("device") == options.end()) {
        throw args::RequiredError("--input i2c: missing `device` option");
    }
    if (options.find("address") == options.end()) {
        throw args::RequiredError("--input i2c: missing `address` option");
    }

    auto device  = options.at("device");
    auto address = 0;
    try {
        address = std::stoi(options.at("address"));
    } catch (...) {
        throw args::ParseError("--input i2c: `address` must be an integer, got `" +
                               options.at("address") + "'");
    }

    // TODO: Implement
    throw args::RequiredError("Not implemented");
    return {};
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

    if (parts[0] == "stdin") return parse_input_stdin(options);
    if (parts[0] == "file") return parse_input_file(options);
    if (parts[0] == "serial") return parse_serial(options);
    if (parts[0] == "tcp-client") return parse_tcp_client(options);
    if (parts[0] == "tcp-server") return parse_tcp_server(options);
    if (parts[0] == "udp-client") return parse_udp_client(options);
    if (parts[0] == "udp-server") return parse_udp_server(options);
    if (parts[0] == "unix-socket") return parse_unix_socket(options);
    if (parts[0] == "i2c") return parse_i2c(options);
    throw args::ValidationError("--input: invalid input type, got `" + parts[0] + "`");
}

static void parse(Config* config) {
    for (auto const& input : gArgs.Get()) {
        config->input.inputs.push_back(parse_interface(input));
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

static char const* baud_rate_to_str(io::BaudRate baud_rate) {
    switch (baud_rate) {
    case io::BaudRate::BR50: return "50";
    case io::BaudRate::BR75: return "75";
    case io::BaudRate::BR110: return "110";
    case io::BaudRate::BR134: return "134";
    case io::BaudRate::BR150: return "150";
    case io::BaudRate::BR200: return "200";
    case io::BaudRate::BR300: return "300";
    case io::BaudRate::BR600: return "600";
    case io::BaudRate::BR1200: return "1200";
    case io::BaudRate::BR1800: return "1800";
    case io::BaudRate::BR2400: return "2400";
    case io::BaudRate::BR4800: return "4800";
    case io::BaudRate::BR9600: return "9600";
    case io::BaudRate::BR19200: return "19200";
    case io::BaudRate::BR38400: return "38400";
    case io::BaudRate::BR57600: return "57600";
    case io::BaudRate::BR115200: return "115200";
    case io::BaudRate::BR230400: return "230400";
    case io::BaudRate::BR460800: return "460800";
    case io::BaudRate::BR500000: return "500000";
    case io::BaudRate::BR576000: return "576000";
    case io::BaudRate::BR921600: return "921600";
    case io::BaudRate::BR1000000: return "1000000";
    case io::BaudRate::BR1152000: return "1152000";
    case io::BaudRate::BR1500000: return "1500000";
    case io::BaudRate::BR2000000: return "2000000";
    case io::BaudRate::BR2500000: return "2500000";
    case io::BaudRate::BR3000000: return "3000000";
    case io::BaudRate::BR3500000: return "3500000";
    case io::BaudRate::BR4000000: return "4000000";
    }
    return "unknown";
}

static char const* data_bits_to_str(io::DataBits data_bits) {
    switch (data_bits) {
    case io::DataBits::FIVE: return "5";
    case io::DataBits::SIX: return "6";
    case io::DataBits::SEVEN: return "7";
    case io::DataBits::EIGHT: return "8";
    }
    return "unknown";
}

static char const* stop_bits_to_str(io::StopBits stop_bits) {
    switch (stop_bits) {
    case io::StopBits::ONE: return "1";
    case io::StopBits::TWO: return "2";
    }
    return "unknown";
}

static char const* parity_bit_to_str(io::ParityBit parity_bit) {
    switch (parity_bit) {
    case io::ParityBit::NONE: return "none";
    case io::ParityBit::ODD: return "odd";
    case io::ParityBit::EVEN: return "even";
    }
    return "unknown";
}

static void dump(InputConfig const& config) {
    for (auto const& input : config.inputs) {
        DEBUGF("%p: %s", input.interface.get(), input_type(input.interface.get()));
        LOGLET_DINDENT_SCOPE();
        DEBUGF("format: %s%s%s%s%s", (input.format & INPUT_FORMAT_UBX) ? "UBX " : "",
               (input.format & INPUT_FORMAT_NMEA) ? "NMEA " : "",
               (input.format & INPUT_FORMAT_RTCM) ? "RTCM " : "",
               (input.format & INPUT_FORMAT_CTRL) ? "CTRL " : "",
               (input.format & INPUT_FORMAT_LPP) ? "LPP " : "");
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
            DEBUGF("baudrate: %s", baud_rate_to_str(serial_input->baud_rate()));
            DEBUGF("data: %s", data_bits_to_str(serial_input->data_bits()));
            DEBUGF("stop: %s", stop_bits_to_str(serial_input->stop_bits()));
            DEBUGF("parity: %s", parity_bit_to_str(serial_input->parity_bit()));
            continue;
        }

        DEBUGF("unimplemented type");
    }
}

}  // namespace input
