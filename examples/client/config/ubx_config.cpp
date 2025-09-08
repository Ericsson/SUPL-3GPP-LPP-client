#include "../config.hpp"
#include <format/ubx/cfg.hpp>
#include <loglet/loglet.hpp>
#include <io/serial.hpp>
#include <io/tcp.hpp>
#include <io/file.hpp>
#include <fstream>
#include <sstream>

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

LOGLET_MODULE(ubx_config);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(ubx_config)

namespace ubx_config {

args::Group gGroup{"Configuration:"};
static args::Group gUbxGroup{gGroup, "UBX:"};

static args::ValueFlagList<std::string> gInterfaces{
    gUbxGroup,
    "interface",
    "Add UBX configuration interface.\n"
    "Usage: --cfg-ubx <type>:<arguments>\n\n"
    "Arguments:\n"
    "  options=<key>=<value>[+<key>=<value>...]\n"
    "  file=<path>\n"
    "  print=<options|all>\n\n"
    "Types and their specific arguments:\n"
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
    "  file:\n"
    "    path=<path>\n"
    "\n"
    "Example: --cfg-ubx serial:device=/dev/ttyUSB0,options=CFG_KEY_RATE_MEAS=1000+CFG_KEY_UART1_ENABLED=true\n"
    "Example: --cfg-ubx serial:device=/dev/ttyUSB0,file=/path/to/ubx_config.txt,print=options",
    {"cfg-ubx"}
};

static args::Flag gApplyAndExit{
    gGroup,
    "apply-and-exit",
    "Apply configuration and exit",
    {"cfg-apply-and-exit"}
};

static void setup() {}

static std::vector<std::string> split(std::string const& str, char delim) {
    std::vector<std::string> tokens;
    std::string              token;
    std::istringstream       token_stream(str);
    while (std::getline(token_stream, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

static format::ubx::CfgKey parse_cfg_key(const std::string& key_str) {
    // Map string keys to actual CFG_KEY values
    if (key_str == "CFG_KEY_RATE_MEAS") return format::ubx::CFG_KEY_RATE_MEAS;
    if (key_str == "CFG_KEY_NAVHPG_DGNSSMODE") return format::ubx::CFG_KEY_NAVHPG_DGNSSMODE;
    if (key_str == "CFG_KEY_UART1_ENABLED") return format::ubx::CFG_KEY_UART1_ENABLED;
    if (key_str == "CFG_KEY_UART1_BAUDRATE") return format::ubx::CFG_KEY_UART1_BAUDRATE;
    if (key_str == "CFG_KEY_UART1_STOPBITS") return format::ubx::CFG_KEY_UART1_STOPBITS;
    if (key_str == "CFG_KEY_UART1_DATABITS") return format::ubx::CFG_KEY_UART1_DATABITS;
    if (key_str == "CFG_KEY_UART1_PARITY") return format::ubx::CFG_KEY_UART1_PARITY;
    if (key_str == "CFG_KEY_UART1INPROT_UBX") return format::ubx::CFG_KEY_UART1INPROT_UBX;
    if (key_str == "CFG_KEY_UART1INPROT_NMEA") return format::ubx::CFG_KEY_UART1INPROT_NMEA;
    if (key_str == "CFG_KEY_UART1INPROT_RTCM3X") return format::ubx::CFG_KEY_UART1INPROT_RTCM3X;
    if (key_str == "CFG_KEY_UART1INPROT_SPARTN") return format::ubx::CFG_KEY_UART1INPROT_SPARTN;
    if (key_str == "CFG_KEY_UART1OUTPROT_UBX") return format::ubx::CFG_KEY_UART1OUTPROT_UBX;
    if (key_str == "CFG_KEY_UART1OUTPROT_NMEA") return format::ubx::CFG_KEY_UART1OUTPROT_NMEA;
    if (key_str == "CFG_KEY_UART1OUTPROT_RTCM3X") return format::ubx::CFG_KEY_UART1OUTPROT_RTCM3X;
    if (key_str == "CFG_KEY_MSGOUT_NAV_PVT_UART1") return format::ubx::CFG_KEY_MSGOUT_NAV_PVT_UART1;
    if (key_str == "CFG_KEY_INFMSG_UART1") return format::ubx::CFG_KEY_INFMSG_UART1;
    
    // Try parsing as hex number
    try {
        if (key_str.substr(0, 2) == "0x" || key_str.substr(0, 2) == "0X") {
            return std::stoul(key_str, nullptr, 16);
        }
        return std::stoul(key_str, nullptr, 10);
    } catch (...) {
        throw args::ValidationError("--cfg-ubx: invalid key '" + key_str + "'");
    }
}

static format::ubx::CfgValue parse_cfg_value(format::ubx::CfgKey key, const std::string& value_str) {
    auto type = format::ubx::CfgValue::type_from_key(key);
    
    try {
        switch (type) {
        case format::ubx::CfgValue::Type::L:
            if (value_str == "true" || value_str == "1") return format::ubx::CfgValue::from_l(true);
            if (value_str == "false" || value_str == "0") return format::ubx::CfgValue::from_l(false);
            throw std::invalid_argument("boolean value expected");
        case format::ubx::CfgValue::Type::U1:
            return format::ubx::CfgValue::from_u1(static_cast<uint8_t>(std::stoul(value_str)));
        case format::ubx::CfgValue::Type::U2:
            return format::ubx::CfgValue::from_u2(static_cast<uint16_t>(std::stoul(value_str)));
        case format::ubx::CfgValue::Type::U4:
            return format::ubx::CfgValue::from_u4(std::stoul(value_str));
        case format::ubx::CfgValue::Type::U8:
            return format::ubx::CfgValue::from_u8(std::stoull(value_str));
        default:
            throw std::invalid_argument("unknown type");
        }
    } catch (...) {
        throw args::ValidationError("--cfg-ubx: invalid value '" + value_str + "' for key");
    }
}

static std::pair<format::ubx::CfgKey, format::ubx::CfgValue> parse_option(const std::string& option) {
    auto eq_pos = option.find('=');
    if (eq_pos == std::string::npos) {
        throw args::ValidationError("--cfg-ubx: missing '=' in option '" + option + "'");
    }
    
    auto key_str = option.substr(0, eq_pos);
    auto value_str = option.substr(eq_pos + 1);
    
    auto key = parse_cfg_key(key_str);
    auto value = parse_cfg_value(key, value_str);
    
    return {key, value};
}

static std::vector<std::pair<format::ubx::CfgKey, format::ubx::CfgValue>> load_options_from_file(const std::string& filename) {
    std::vector<std::pair<format::ubx::CfgKey, format::ubx::CfgValue>> options;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        throw args::ValidationError("--cfg-ubx: cannot open file '" + filename + "'");
    }
    
    std::string line;
    int line_num = 0;
    while (std::getline(file, line)) {
        line_num++;
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        try {
            options.push_back(parse_option(line));
        } catch (const args::ValidationError& e) {
            throw args::ValidationError("--cfg-ubx file line " + std::to_string(line_num) + ": " + e.what());
        }
    }
    
    return options;
}

static std::vector<std::pair<format::ubx::CfgKey, format::ubx::CfgValue>> parse_options_list(const std::string& options_str) {
    std::vector<std::pair<format::ubx::CfgKey, format::ubx::CfgValue>> options;
    auto option_parts = split(options_str, '+');
    
    for (const auto& option : option_parts) {
        options.push_back(parse_option(option));
    }
    
    return options;
}

static std::unordered_map<std::string, std::string> parse_arguments(std::string const& str) {
    auto parts = split(str, ',');
    std::unordered_map<std::string, std::string> arguments;
    for (auto const& part : parts) {
        auto kv = split(part, '=');
        if (kv.size() != 2) {
            throw args::ValidationError("--cfg-ubx: invalid argument, got `" + part + "`");
        }
        arguments[kv[0]] = kv[1];
    }
    return arguments;
}

static io::BaudRate parse_baudrate(std::string const& str) {
    long baud_rate = 0;
    try {
        baud_rate = std::stol(str);
    } catch (...) {
        throw args::ParseError("--cfg-ubx serial: `baudrate` must be an integer, got `" + str + "'");
    }

    if (baud_rate == 9600) return io::BaudRate::BR9600;
    if (baud_rate == 19200) return io::BaudRate::BR19200;
    if (baud_rate == 38400) return io::BaudRate::BR38400;
    if (baud_rate == 57600) return io::BaudRate::BR57600;
    if (baud_rate == 115200) return io::BaudRate::BR115200;
    if (baud_rate == 230400) return io::BaudRate::BR230400;
    if (baud_rate == 460800) return io::BaudRate::BR460800;
    if (baud_rate == 921600) return io::BaudRate::BR921600;
    throw args::ParseError("--cfg-ubx serial: `baudrate` must be a valid baud rate, got `" + str + "'");
}

static io::DataBits parse_databits(std::string const& str) {
    long databits = 0;
    try {
        databits = std::stol(str);
    } catch (...) {
        throw args::ParseError("--cfg-ubx serial: `databits` must be an integer, got `" + str + "'");
    }

    if (databits == 5) return io::DataBits::FIVE;
    if (databits == 6) return io::DataBits::SIX;
    if (databits == 7) return io::DataBits::SEVEN;
    if (databits == 8) return io::DataBits::EIGHT;
    throw args::ParseError("--cfg-ubx serial: `databits` must be 5, 6, 7, or 8, got `" + str + "'");
}

static io::StopBits parse_stopbits(std::string const& str) {
    long stopbits = 0;
    try {
        stopbits = std::stol(str);
    } catch (...) {
        throw args::ParseError("--cfg-ubx serial: `stopbits` must be an integer, got `" + str + "'");
    }

    if (stopbits == 1) return io::StopBits::ONE;
    if (stopbits == 2) return io::StopBits::TWO;
    throw args::ParseError("--cfg-ubx serial: `stopbits` must be 1 or 2, got `" + str + "'");
}

static io::ParityBit parse_paritybit(std::string const& str) {
    if (str == "none") return io::ParityBit::NONE;
    if (str == "odd") return io::ParityBit::ODD;
    if (str == "even") return io::ParityBit::EVEN;
    throw args::ParseError("--cfg-ubx serial: `parity` must be none, odd, or even, got `" + str + "'");
}

static std::pair<std::unique_ptr<io::Output>, std::unique_ptr<io::Input>> parse_serial_interfaces(std::unordered_map<std::string, std::string> const& arguments) {
    if (arguments.find("device") == arguments.end()) {
        throw args::RequiredError("--cfg-ubx serial: missing `device` argument");
    }

    auto baud_rate = io::BaudRate::BR115200;
    auto data_bits = io::DataBits::EIGHT;
    auto stop_bits = io::StopBits::ONE;
    auto parity_bit = io::ParityBit::NONE;

    auto baud_rate_it = arguments.find("baudrate");
    if (baud_rate_it != arguments.end()) baud_rate = parse_baudrate(baud_rate_it->second);

    auto data_bits_it = arguments.find("databits");
    if (data_bits_it != arguments.end()) data_bits = parse_databits(data_bits_it->second);

    auto stop_bits_it = arguments.find("stopbits");
    if (stop_bits_it != arguments.end()) stop_bits = parse_stopbits(stop_bits_it->second);

    auto parity_bit_it = arguments.find("parity");
    if (parity_bit_it != arguments.end()) parity_bit = parse_paritybit(parity_bit_it->second);

    auto device = arguments.at("device");
    auto output = std::unique_ptr<io::Output>(new io::SerialOutput(device, baud_rate, data_bits, stop_bits, parity_bit));
    auto input = std::unique_ptr<io::Input>(new io::SerialInput(device, baud_rate, data_bits, stop_bits, parity_bit));
    return {std::move(output), std::move(input)};
}

static std::pair<std::unique_ptr<io::Output>, std::unique_ptr<io::Input>> parse_tcp_client_interfaces(std::unordered_map<std::string, std::string> const& arguments) {
    if (arguments.find("host") != arguments.end()) {
        if (arguments.find("port") == arguments.end()) {
            throw args::RequiredError("--cfg-ubx tcp-client: missing `port` argument");
        }

        auto host = arguments.at("host");
        auto port = 0;
        try {
            port = std::stoi(arguments.at("port"));
        } catch (...) {
            throw args::ParseError("--cfg-ubx tcp-client: `port` must be an integer, got `" +
                                   arguments.at("port") + "'");
        }

        if (port < 0 || port > 65535) {
            throw args::ParseError(
                "--cfg-ubx tcp-client: `port` must be in the range [0, 65535], got `" +
                std::to_string(port) + "'");
        }

        auto output = std::unique_ptr<io::Output>(new io::TcpClientOutput(host, static_cast<uint16_t>(port), false));
        auto input = std::unique_ptr<io::Input>(new io::TcpClientInput(host, static_cast<uint16_t>(port), false));
        return {std::move(output), std::move(input)};
    } else if (arguments.find("path") != arguments.end()) {
        auto path = arguments.at("path");
        auto output = std::unique_ptr<io::Output>(new io::TcpClientOutput(path, false));
        auto input = std::unique_ptr<io::Input>(new io::TcpClientInput(path, false));
        return {std::move(output), std::move(input)};
    } else {
        throw args::RequiredError("--cfg-ubx tcp-client: missing `host` and `port` or `path` argument");
    }
}

static std::pair<std::unique_ptr<io::Output>, std::unique_ptr<io::Input>> parse_file_interfaces(std::unordered_map<std::string, std::string> const& arguments) {
    if (arguments.find("path") == arguments.end()) {
        throw args::RequiredError("--cfg-ubx file: missing `path` argument");
    }

    auto path = arguments.at("path");
    auto output = std::unique_ptr<io::Output>(new io::FileOutput(path, false, true, true));
    auto input = std::unique_ptr<io::Input>(new io::FileInput(path, 1024, std::chrono::milliseconds(100)));
    return {std::move(output), std::move(input)};
}

static UbxPrintMode parse_print_mode(const std::string& print_str) {
    if (print_str == "options") return UbxPrintMode::OPTIONS;
    if (print_str == "all") return UbxPrintMode::ALL;
    throw args::ValidationError("--cfg-ubx: invalid print mode, expected 'options' or 'all', got '" + print_str + "'");
}

static UbxConfigInterface parse_interface(std::string const& source) {
    auto parts = split(source, ':');
    if (parts.size() != 2) {
        throw args::ValidationError("--cfg-ubx: invalid format, expected <type>:<arguments>");
    }

    auto type = parts[0];
    auto arguments = parse_arguments(parts[1]);

    std::unique_ptr<io::Output> output;
    std::unique_ptr<io::Input> input;
    
    if (type == "serial") {
        auto interfaces = parse_serial_interfaces(arguments);
        output = std::move(interfaces.first);
        input = std::move(interfaces.second);
    } else if (type == "tcp-client") {
        auto interfaces = parse_tcp_client_interfaces(arguments);
        output = std::move(interfaces.first);
        input = std::move(interfaces.second);
    } else if (type == "file") {
        auto interfaces = parse_file_interfaces(arguments);
        output = std::move(interfaces.first);
        input = std::move(interfaces.second);
    } else {
        throw args::ValidationError("--cfg-ubx: invalid type, got `" + type + "`");
    }

    std::vector<std::pair<format::ubx::CfgKey, format::ubx::CfgValue>> options;

    if (arguments.find("options") != arguments.end()) {
        options = parse_options_list(arguments.at("options"));
    }

    if (arguments.find("file") != arguments.end()) {
        auto file_options = load_options_from_file(arguments.at("file"));
        options.insert(options.end(), file_options.begin(), file_options.end());
    }

    UbxPrintMode print_mode = UbxPrintMode::NONE;
    if (arguments.find("print") != arguments.end()) {
        print_mode = parse_print_mode(arguments.at("print"));
    }

    if (options.empty() && print_mode == UbxPrintMode::NONE) {
        throw args::ValidationError("--cfg-ubx: no options specified and no print mode (use 'options=', 'file=', or 'print=')");
    }

    UbxConfigInterface result;
    result.output_interface = std::move(output);
    result.input_interface = std::move(input);
    result.options = std::move(options);
    result.print_mode = print_mode;
    return result;
}

static void parse(Config* config) {
    for (const auto& interface_str : gInterfaces.Get()) {
        config->ubx_config.interfaces.push_back(parse_interface(interface_str));
    }
    
    config->ubx_config.apply_and_exit = gApplyAndExit.Get();
}

static void dump(const UbxConfigConfig& config) {
    DEBUGF("interfaces count: %zu", config.interfaces.size());
    DEBUGF("apply_and_exit: %s", config.apply_and_exit ? "true" : "false");
    
    for (size_t i = 0; i < config.interfaces.size(); i++) {
        const auto& interface = config.interfaces[i];
        DEBUGF("interface %zu:", i);
        DEBUG_INDENT_SCOPE();
        DEBUGF("output: %s", interface.output_interface->name());
        DEBUGF("options count: %zu", interface.options.size());
        
        std::string print_mode_str;
        switch (interface.print_mode) {
        case UbxPrintMode::NONE: print_mode_str = "none"; break;
        case UbxPrintMode::OPTIONS: print_mode_str = "options"; break;
        case UbxPrintMode::ALL: print_mode_str = "all"; break;
        }
        DEBUGF("print mode: %s", print_mode_str.c_str());
        
        for (const auto& [key, value] : interface.options) {
            std::string value_str;
            switch (value.type()) {
            case format::ubx::CfgValue::Type::L:
                value_str = value.l() ? "true" : "false";
                break;
            case format::ubx::CfgValue::Type::U1:
                value_str = std::to_string(value.u1());
                break;
            case format::ubx::CfgValue::Type::U2:
                value_str = std::to_string(value.u2());
                break;
            case format::ubx::CfgValue::Type::U4:
                value_str = std::to_string(value.u4());
                break;
            case format::ubx::CfgValue::Type::U8:
                value_str = std::to_string(value.u8());
                break;
            default:
                value_str = "unknown";
                break;
            }
            DEBUGF("  0x%08X = %s", key, value_str.c_str());
        }
    }
}

}  // namespace ubx_config