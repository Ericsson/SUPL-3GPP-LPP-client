
namespace output {

static args::Group                      gGroup{"Output:"};
static args::ValueFlagList<std::string> gArgs{
    gGroup,
    "output",
    "Add an output interface.\n"
    "Usage: --output <type>:<arguments>\n\n"
    "Arguments:\n"
    "  format=<fmt>[+<fmt>...]\n\n"
    "Types and their specific arguments:\n"
    "  stdout:\n"
    "  file:\n"
    "    path=<path>\n"
    "    append=<true|false>\n"
    "  serial:\n"
    "    device=<device>\n"
    "    baudrate=<baudrate>\n"
    "    databits=<5|6|7|8>\n"
    "    stopbits=<1|2>\n"
    "    parity=<none|odd|even>\n"
    "  tcp-client:\n"
    "    host=<host>\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "  udp-client:\n"
    "    host=<host>\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "\n"
    "Formats:\n"
    "  all, ubx, nmea, rtcm, ctrl, spartn, lpp-xer, lpp-uper\n"
    "Examples:\n"
    "  --output file:path=/tmp/output,format=ubx+nmea",
    {"output"},
};

static void setup() {}

static bool parse_bool_option(std::unordered_map<std::string, std::string> const& options,
                              std::string const& type, std::string const& key, bool default_value) {
    if (options.find(key) == options.end()) return default_value;
    auto value = options.at(key);
    if (value == "true") return true;
    if (value == "false") return false;
    throw args::ValidationError("--output " + type + ": `" + key + "` must be a boolean, got `" +
                                value + "'");
}

static OutputFormat parse_format(std::string const& str) {
    if (str == "all") return OUTPUT_FORMAT_ALL;
    if (str == "ubx") return OUTPUT_FORMAT_UBX;
    if (str == "nmea") return OUTPUT_FORMAT_NMEA;
    if (str == "rtcm") return OUTPUT_FORMAT_RTCM;
    if (str == "ctrl") return OUTPUT_FORMAT_CTRL;
    if (str == "spartn") return OUTPUT_FORMAT_SPARTN;
    if (str == "lpp-xer") return OUTPUT_FORMAT_LPP_XER;
    if (str == "lpp-uper") return OUTPUT_FORMAT_LPP_UPER;
    throw args::ValidationError("--output format: invalid format, got `" + str + "`");
}

static OutputFormat parse_format_list(std::string const& str) {
    auto parts  = split(str, '+');
    auto format = OUTPUT_FORMAT_NONE;
    for (auto const& part : parts) {
        format |= parse_format(part);
    }
    return format;
}

static OutputFormat
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

static OutputInterface parse_stdout(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "stdin", "print", false);
    auto output = std::unique_ptr<io::Output>(new io::StdoutOutput());
    return {format, std::move(output), print};
}

static OutputInterface parse_file(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "file", "print", false);
    if (options.find("path") == options.end()) {
        throw args::ValidationError("--output file: missing `path` option");
    }
    auto path = options.at("path");

    auto truncate = true;
    auto append   = false;
    if (options.find("append") != options.end()) {
        if (options.at("append") == "true") {
            append   = true;
            truncate = false;
        } else if (options.at("append") == "false") {
        } else {
            throw args::ValidationError("--output file: `append` must be a boolean, got `" +
                                        options.at("append") + "'");
        }
    }
    auto output = std::unique_ptr<io::Output>(new io::FileOutput(path, truncate, append, true));
    return {format, std::move(output), print};
}

static io::BaudRate parse_baudrate(std::string const& str) {
    long baud_rate = 0;
    try {
        baud_rate = std::stol(str);
    } catch (...) {
        throw args::ParseError("--output serial: `baudrate` must be an integer, got `" + str + "'");
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
    throw args::ParseError("--output serial: `baudrate` must be a valid baud rate, got `" + str +
                           "'");
}

static io::DataBits parse_databits(std::string const& str) {
    long databits = 0;
    try {
        databits = std::stol(str);
    } catch (...) {
        throw args::ParseError("--output serial: `data` must be an integer, got `" + str + "'");
    }

    if (databits == 5) return io::DataBits::FIVE;
    if (databits == 6) return io::DataBits::SIX;
    if (databits == 7) return io::DataBits::SEVEN;
    if (databits == 8) return io::DataBits::EIGHT;
    throw args::ParseError("--output serial: `data` must be 5, 6, 7, or 8, got `" + str + "'");
}

static io::StopBits parse_stopbits(std::string const& str) {
    long stopbits = 0;
    try {
        stopbits = std::stol(str);
    } catch (...) {
        throw args::ParseError("--output serial: `stop` must be an integer, got `" + str + "'");
    }

    if (stopbits == 1) return io::StopBits::ONE;
    if (stopbits == 2) return io::StopBits::TWO;
    throw args::ParseError("--output serial: `stop` must be 1 or 2, got `" + str + "'");
}

static io::ParityBit parse_paritybit(std::string const& str) {
    if (str == "none") return io::ParityBit::NONE;
    if (str == "odd") return io::ParityBit::ODD;
    if (str == "even") return io::ParityBit::EVEN;
    throw args::ParseError("--output serial: `parity` must be none, odd, or even, got `" + str +
                           "'");
}

static OutputInterface parse_serial(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "serial", "print", false);
    if (options.find("device") == options.end()) {
        throw args::RequiredError("--output serial: missing `device` option");
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
    auto output = std::unique_ptr<io::Output>(
        new io::SerialOutput(device, baud_rate, data_bits, stop_bits, parity_bit));
    return {format, std::move(output), print};
}

static OutputInterface
parse_tcp_client(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "tcp", "print", false);

    auto reconnect = true;
    if (options.find("reconnect") != options.end()) {
        if (options.at("reconnect") == "true") {
            reconnect = true;
        } else if (options.at("reconnect") == "false") {
            reconnect = false;
        } else {
            throw args::ValidationError(
                "--output udp-client: `reconnect` must be a boolean, got `" +
                options.at("reconnect") + "'");
        }
    }

    if (options.find("host") != options.end()) {
        if (options.find("host") == options.end()) {
            throw args::RequiredError("--output tcp-client: missing `host` option");
        }
        if (options.find("port") == options.end()) {
            throw args::RequiredError("--output tcp-client: missing `port` option");
        }
        if (options.find("path") != options.end()) {
            throw args::RequiredError(
                "--output tcp-client: `path` cannot be used with `host` and `port`");
        }

        auto host = options.at("host");
        auto port = 0;
        try {
            port = std::stoi(options.at("port"));
        } catch (...) {
            throw args::ParseError("--output tcp-client: `port` must be an integer, got `" +
                                   options.at("port") + "'");
        }

        return {format, std::unique_ptr<io::Output>(new io::TcpClientOutput(host, port, reconnect)),
                print};
    } else if (options.find("path") != options.end()) {
        auto path = options.at("path");
        return {format, std::unique_ptr<io::Output>(new io::TcpClientOutput(path, reconnect)),
                print};
    } else {
        throw args::RequiredError(
            "--output tcp-client: missing `host` and `port` or `path` option");
    }
}

static OutputInterface
parse_udp_client(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_format_list_from_options(options);
    auto print  = parse_bool_option(options, "udp-client", "print", false);

    if (options.find("host") != options.end()) {
        if (options.find("host") == options.end()) {
            throw args::RequiredError("--output udp-client: missing `host` option");
        }
        if (options.find("port") == options.end()) {
            throw args::RequiredError("--output udp-client: missing `port` option");
        }
        if (options.find("path") != options.end()) {
            throw args::RequiredError(
                "--output udp-client: `path` cannot be used with `host` and `port`");
        }

        auto host = options.at("host");
        auto port = 0;
        try {
            port = std::stoi(options.at("port"));
        } catch (...) {
            throw args::ParseError("--output udp-client: `port` must be an integer, got `" +
                                options.at("port") + "'");
        }

        return {format, std::unique_ptr<io::Output>(new io::UdpClientOutput(host, port)), print};
    } else if (options.find("path") != options.end()) {
        auto path = options.at("path");
        return {format, std::unique_ptr<io::Output>(new io::UdpClientOutput(path)), print};
    } else {
        throw args::RequiredError(
            "--output udp-client: missing `host` and `port` or `path` option");
    }
}

static OutputInterface parse_interface(std::string const& source) {
    std::unordered_map<std::string, std::string> options;

    auto parts = split(source, ':');
    if (parts.size() == 1) {
        // No options
    } else if (parts.size() == 2) {
        options = parse_options(parts[1]);
    } else {
        throw args::ParseError("--output not in type:arguments format: \"" + source + "\"");
    }

    if (parts[0] == "stdout") return parse_stdout(options);
    if (parts[0] == "file") return parse_file(options);
    if (parts[0] == "serial") return parse_serial(options);
    if (parts[0] == "tcp-client") return parse_tcp_client(options);
    if (parts[0] == "udp-client") return parse_udp_client(options);
    throw args::ParseError("--output type not recognized: \"" + parts[0] + "\"");
}

static void parse(Config* config) {
    for (auto const& output : gArgs.Get()) {
        config->output.outputs.push_back(parse_interface(output));
    }
}

static char const* output_type(io::Output* output) {
    if (dynamic_cast<io::StdoutOutput*>(output)) return "stdout";
    if (dynamic_cast<io::FileOutput*>(output)) return "file";
    if (dynamic_cast<io::SerialOutput*>(output)) return "serial";
    return "unknown";
}

static void dump(OutputConfig const& config) {
    for (auto const& output : config.outputs) {
        DEBUGF("%p: %s", output.interface.get(), output_type(output.interface.get()));
        LOGLET_DINDENT_SCOPE();
        DEBUGF("format: %s%s%s%s%s%s%s", (output.format & OUTPUT_FORMAT_UBX) ? "UBX " : "",
               (output.format & OUTPUT_FORMAT_NMEA) ? "NMEA " : "",
               (output.format & OUTPUT_FORMAT_RTCM) ? "RTCM " : "",
               (output.format & OUTPUT_FORMAT_CTRL) ? "CTRL " : "",
               (output.format & OUTPUT_FORMAT_LPP_XER) ? "LPP-XER " : "",
               (output.format & OUTPUT_FORMAT_LPP_UPER) ? "LPP-UPER " : "",
               (output.format & OUTPUT_FORMAT_SPARTN) ? "SPARTN " : "");

        auto stdout_output = dynamic_cast<io::StdoutOutput*>(output.interface.get());
        if (stdout_output) continue;

        auto file_output = dynamic_cast<io::FileOutput*>(output.interface.get());
        if (file_output) {
            DEBUGF("path: %s", file_output->path().c_str());
            DEBUGF("append: %s", file_output->append() ? "true" : "false");
            continue;
        }

        auto serial_output = dynamic_cast<io::SerialOutput*>(output.interface.get());
        if (serial_output) {
            DEBUGF("device: %s", serial_output->device().c_str());
            DEBUGF("baudrate: %s", io::baud_rate_to_string(serial_output->baud_rate()));
            DEBUGF("data bits: %s", io::data_bits_to_string(serial_output->data_bits()));
            DEBUGF("stop bits: %s", io::stop_bits_to_string(serial_output->stop_bits()));
            DEBUGF("parity bit: %s", io::parity_bit_to_string(serial_output->parity_bit()));
            continue;
        }

        auto tcp_client_output = dynamic_cast<io::TcpClientOutput*>(output.interface.get());
        if (tcp_client_output) {
            DEBUGF("host: %s", tcp_client_output->host().c_str());
            DEBUGF("port: %d", tcp_client_output->port());
            DEBUGF("reconnect: %s", tcp_client_output->reconnect() ? "true" : "false");
            continue;
        }

        auto udp_client_output = dynamic_cast<io::UdpClientOutput*>(output.interface.get());
        if (udp_client_output) {
            DEBUGF("host: %s", udp_client_output->host().c_str());
            DEBUGF("port: %d", udp_client_output->port());
            continue;
        }

        DEBUGF("unimplemented type");
    }
}

}  // namespace output
