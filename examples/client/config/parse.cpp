#include "parse.hpp"

#include <core/string.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

Options parse_options(std::string const& str) {
    auto    parts = core::split(str, ',');
    Options options;
    for (auto const& part : parts) {
        auto kv = core::split(part, '=');
        if (kv.size() == 1) {
            options[kv[0]] = "";
        } else if (kv.size() == 2) {
            options[kv[0]] = kv[1];
        } else {
            throw args::ValidationError("invalid option: `" + part + "`");
        }
    }
    return options;
}

bool parse_bool(Options const& options, std::string const& key, bool default_value) {
    auto it = options.find(key);
    if (it == options.end()) return default_value;
    if (it->second.empty()) return true;
    if (it->second == "true") return true;
    if (it->second == "false") return false;
    throw args::ValidationError("`" + key + "` must be a boolean, got `" + it->second + "`");
}

io::BaudRate parse_baudrate(std::string const& str) {
    long baud_rate = 0;
    try {
        baud_rate = std::stol(str);
    } catch (...) {
        throw args::ParseError("`baudrate` must be an integer, got `" + str + "`");
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
    throw args::ParseError("`baudrate` must be a valid baud rate, got `" + str + "`");
}

io::DataBits parse_databits(std::string const& str) {
    long databits = 0;
    try {
        databits = std::stol(str);
    } catch (...) {
        throw args::ParseError("`databits` must be an integer, got `" + str + "`");
    }

    if (databits == 5) return io::DataBits::FIVE;
    if (databits == 6) return io::DataBits::SIX;
    if (databits == 7) return io::DataBits::SEVEN;
    if (databits == 8) return io::DataBits::EIGHT;
    throw args::ParseError("`databits` must be 5, 6, 7, or 8, got `" + str + "`");
}

io::StopBits parse_stopbits(std::string const& str) {
    long stopbits = 0;
    try {
        stopbits = std::stol(str);
    } catch (...) {
        throw args::ParseError("`stopbits` must be an integer, got `" + str + "`");
    }

    if (stopbits == 1) return io::StopBits::ONE;
    if (stopbits == 2) return io::StopBits::TWO;
    throw args::ParseError("`stopbits` must be 1 or 2, got `" + str + "`");
}

io::ParityBit parse_paritybit(std::string const& str) {
    if (str == "none") return io::ParityBit::NONE;
    if (str == "odd") return io::ParityBit::ODD;
    if (str == "even") return io::ParityBit::EVEN;
    throw args::ParseError("`parity` must be none, odd, or even, got `" + str + "`");
}

io::ReadBufferConfig parse_read_config(Options const& options) {
    io::ReadBufferConfig config;
    auto                 it = options.find("read_min_bytes");
    if (it != options.end()) {
        config.min_bytes = std::stoull(it->second);
    }
    it = options.find("read_timeout_ms");
    if (it != options.end()) {
        config.timeout = std::chrono::milliseconds(std::stoll(it->second));
    }
    return config;
}

std::vector<std::string> parse_list(std::string const& str, char delimiter) {
    return core::split(str, delimiter);
}

std::string generate_unique_id() {
    static int counter = 0;
    return "auto-" + std::to_string(++counter);
}
