#include "options.hpp"
#include <io/file.hpp>
#include <io/serial.hpp>
#include <io/stdin.hpp>
#include <io/stdout.hpp>
#include <io/tcp.hpp>
#include <io/udp.hpp>

#include <math.h>

static std::vector<std::string> split(std::string const& str, char delim) {
    std::vector<std::string> tokens;
    std::string              token;
    std::istringstream       token_stream(str);
    while (std::getline(token_stream, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

static std::vector<std::string> split_at_any(std::string const& input,
                                             std::string const& delimiters) {
    std::vector<std::string> result;
    std::size_t              last_pos = 0;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (delimiters.find(input[i]) != delimiters.npos) {
            result.push_back(input.substr(last_pos, i - last_pos));
            ++i;
            last_pos = i - 1;
        }
    }

    if (last_pos != input.size()) {
        result.push_back(input.substr(last_pos, input.size()));
    }

    return result;
}

static args::Group gArguments{"Arguments:"};

//
// Location Server
//
static args::Group gLocationServer{
    "Location Server:",
    args::Group::Validators::All,
    args::Options::Global,
};

static args::ValueFlag<std::string> gLocationServerHost{
    gLocationServer, "host", "Host", {'h', "host"}, args::Options::Single};
static args::ValueFlag<uint16_t> gLocationServerPort{
    gLocationServer, "port", "Port", {'p', "port"}, args::Options::Single};
static args::Flag gLocationServerSsl{
    gLocationServer, "ssl", "TLS", {'s', "ssl"}, args::Options::Single};
static args::Flag gLocationServerSlpHostCell{gLocationServer,
                                             "slp-host-cell",
                                             "Use Cell ID as SLP Host",
                                             {"slp-host-cell"},
                                             args::Options::Single};
static args::Flag gLocationServerSlpHostImsi{gLocationServer,
                                             "slp-host-imsi",
                                             "Use IMSI as SLP Host",
                                             {"slp-host-imsi"},
                                             args::Options::Single};
static args::Flag gSkipConnect{
    gLocationServer, "skip-connect", "Skip Connect", {"skip-connect"}, args::Options::Single};
static args::Flag gSkipRequestAssistanceData{gLocationServer,
                                             "skip-request-assistance-data",
                                             "Skip Request Assistance Data",
                                             {"skip-request-assistance-data"},
                                             args::Options::Single};

//
// Identity
//
static args::Group gIdentity{
    "Identity:",
    args::Group::Validators::All,
    args::Options::Global,
};

static args::ValueFlag<unsigned long long> gMsisdn{
    gIdentity, "msisdn", "MSISDN", {"msisdn"}, args::Options::Single};
static args::ValueFlag<unsigned long long> gImsi{
    gIdentity, "imsi", "IMSI", {"imsi"}, args::Options::Single};
static args::ValueFlag<std::string> gIpv4{
    gIdentity, "ipv4", "IPv4", {"ipv4"}, args::Options::Single};
static args::Flag gUseSuplIdentityFix{gIdentity,
                                      "supl-identity-fix",
                                      "Use SUPL Identity Fix",
                                      {"supl-identity-fix"},
                                      args::Options::Single};
static args::Flag gWaitForIdentity{gIdentity,
                                   "wait-for-identity",
                                   "Wait for the identity to be provided via the control interface",
                                   {"wait-for-identity"},
                                   args::Options::Single};

//
// Cell Information
//
static args::Group gCellInformation{
    "Cell Information:",
    args::Group::Validators::All,
    args::Options::Global,
};

static args::ValueFlag<int>                gMcc{gCellInformation,
                                 "mcc",
                                 "Mobile Country Code",
                                                {'c', "mcc"},
                                 args::Options::Single | args::Options::Required};
static args::ValueFlag<int>                gMnc{gCellInformation,
                                 "mnc",
                                 "Mobile Network Code",
                                                {'n', "mnc"},
                                 args::Options::Single | args::Options::Required};
static args::ValueFlag<int>                gTac{gCellInformation,
                                 "tac",
                                 "Tracking Area Code",
                                                {'t', "lac", "tac"},
                                 args::Options::Single | args::Options::Required};
static args::ValueFlag<unsigned long long> gCi{gCellInformation,
                                               "ci",
                                               "Cell Identity",
                                               {'i', "ci"},
                                               args::Options::Single | args::Options::Required};
static args::Flag gIsNr{gCellInformation, "nr", "The cell specified is a 5G NR cell", {"nr"}};

//
// u-blox
//

static args::Group gUbloxReceiverGroup{
    "u-blox Receiver:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlag<std::string> gUbloxReceiverPort{
    gUbloxReceiverGroup,
    "port",
    "The port used on the u-blox receiver, used by configuration.",
    {"ublox-port"},
    args::Options::Single};

static args::Group gUbloxSerialGroup{
    gUbloxReceiverGroup,
    "Serial:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> gUbloxSerialDevice{
    gUbloxSerialGroup, "device", "Device", {"ublox-serial"}, args::Options::Single};
static args::ValueFlag<int> gUbloxSerialBaudRate{
    gUbloxSerialGroup, "baud_rate", "Baud Rate", {"ublox-serial-baud"}, args::Options::Single};
static args::ValueFlag<int> gUbloxSerialDataBits{
    gUbloxSerialGroup, "data_bits", "Data Bits", {"ublox-serial-data"}, args::Options::Single};
static args::ValueFlag<int> gUbloxSerialStopBits{
    gUbloxSerialGroup, "stop_bits", "Stop Bits", {"ublox-serial-stop"}, args::Options::Single};
static args::ValueFlag<std::string> gUbloxSerialParityBits{gUbloxSerialGroup,
                                                           "parity_bits",
                                                           "Parity Bits",
                                                           {"ublox-serial-parity"},
                                                           args::Options::Single};

static args::Group gUbloxTcpGroup{
    gUbloxReceiverGroup,
    "TCP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> gUbloxTcpIpAddress{
    gUbloxTcpGroup, "ip_address", "Host or IP Address", {"ublox-tcp"}, args::Options::Single};
static args::ValueFlag<uint16_t> gUbloxTcpPort{
    gUbloxTcpGroup, "port", "Port", {"ublox-tcp-port"}, args::Options::Single};

//
// NMEA
//

static args::Group gNmeaReceiverGroup{
    "NMEA Receiver:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Group gNmeaSerialGroup{
    gNmeaReceiverGroup,
    "Serial:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> gNmeaSerialDevice{
    gNmeaSerialGroup, "device", "Device", {"nmea-serial"}, args::Options::Single};
static args::ValueFlag<int> gNmeaSerialBaudRate{
    gNmeaSerialGroup, "baud_rate", "Baud Rate", {"nmea-serial-baud"}, args::Options::Single};
static args::ValueFlag<int> gNmeaSerialDataBits{
    gNmeaSerialGroup, "data_bits", "Data Bits", {"nmea-serial-data"}, args::Options::Single};
static args::ValueFlag<int> gNmeaSerialStopBits{
    gNmeaSerialGroup, "stop_bits", "Stop Bits", {"nmea-serial-stop"}, args::Options::Single};
static args::ValueFlag<std::string> gNmeaSerialParityBits{
    gNmeaSerialGroup, "parity_bits", "Parity Bits", {"nmea-serial-parity"}, args::Options::Single};

static args::Group gNmeaTcpGroup{
    gNmeaReceiverGroup,
    "TCP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> gNmeaTcpIpAddress{
    gNmeaTcpGroup, "ip_address", "Host or IP Address", {"nmea-tcp"}, args::Options::Single};
static args::ValueFlag<uint16_t> gNmeaTcpPort{
    gNmeaTcpGroup, "port", "Port", {"nmea-tcp-port"}, args::Options::Single};

// export nmea to unix socket
static args::ValueFlag<std::string> gNmeaExportUn{gNmeaReceiverGroup,
                                                  "unix socket",
                                                  "Export NMEA to unix socket",
                                                  {"nmea-export-un"},
                                                  args::Options::Single};
static args::ValueFlag<std::string> gNmeaExportTcp{
    gNmeaReceiverGroup, "ip", "Export NMEA to TCP", {"nmea-export-tcp"}, args::Options::Single};
static args::ValueFlag<uint16_t> gNmeaExportTcpPort{gNmeaReceiverGroup,
                                                    "port",
                                                    "Export NMEA to TCP Port",
                                                    {"nmea-export-tcp-port"},
                                                    args::Options::Single};

//
// Input
//

static args::Group gInputGroup{
    "Input Options:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlagList<std::string> gInputArgs{
    gInputGroup,
    "input",
    "Specify data input(s).\nUsage: --input <type>:<arguments>\n"
    "All types have the following arguments:\n"
    "  format=<fmt>[|<fmt>...]\n"
    "type: stdin\n"
    "type: file\n"
    "  path=<path>\n"
    "type: serial\n"
    "  device=<device>\n"
    "  baudrate=<baudrate>\n"
    "  databits=<5|6|7|8>\n"
    "  stopbits=<1|2>\n"
    "  parity=<none|odd|even>\n"
    "type: tcp-client\n"
    "  host=<host>\n"
    "  port=<port>\n"
    "  reconnect=<true|false>\n"
    "type: tcp-server\n"
    "  port=<port>\n"
    "type: udp-client\n"
    "  host=<host>\n"
    "  port=<port>\n"
    "type: udp-server\n"
    "  port=<port>\n"
    "type: i2c\n"
    "  device=<device>\n"
    "  address=<address>\n"
    "type: unix-socket\n"
    "  path=<path>\n"
    "\n"
    "format:\n"
    "  fmt: [+|-]<name>\n"
    "  name: ubx|nmea|rtcm|control\n",
    {"input"}};

//
// Output
//

static args::Group gOtherReceiverGroup{
    "Other Receiver Options:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Flag gPrintMessages{gOtherReceiverGroup,
                                 "print-receiver-messages",
                                 "Print Receiver Messages",
                                 {"print-receiver-messages", "prm"},
                                 args::Options::Single};

static args::Flag gReceiverReadonly{
    gOtherReceiverGroup, "readonly", "Readonly Receiver", {"readonly"}, args::Options::Single};

//
// Output
//

static args::Group gOutputGroup{
    "Output:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlagList<std::string> gOutputArgs{
    gOutputGroup,
    "output",
    "Specify data output(s).\nUsage: --output <type>:<arguments>\n"
    "All types have the following arguments:\n"
    "  format=<fmt>[+<fmt>...]\n"
    "type: file\n"
    "  path=<path>\n"
    "type: serial\n"
    "  device=<device>\n"
    "  baudrate=<baudrate>\n"
    "  databits=<5|6|7|8>\n"
    "  stopbits=<1|2>\n"
    "  parity=<none|odd|even>\n"
    "type: tcp-client\n"
    "  host=<host>\n"
    "  port=<port>\n"
    "type: udp-client\n"
    "  host=<host>\n"
    "  port=<port>\n"
    "type: stdout\n"
    "\n"
    "format:\n"
    "  fmt: ubx|nmea|rtcm|ctrl|spartn|lpp-xer|lpp-uper\n",
    {"output"}};

static OutputFormat
parse_output_format(std::unordered_map<std::string, std::string> const& options) {
    if (options.find("format") == options.end()) {
        return OUTPUT_FORMAT_NONE;
    }

    auto fmt    = options.at("format");
    auto parts  = split(fmt, '+');
    auto format = OUTPUT_FORMAT_NONE;
    for (auto const& part : parts) {
        if (part.empty()) {
            continue;
        } else if (part.length() < 2) {
            throw args::ParseError("invalid output format format: \"" + part + "\"");
        }

        auto change      = OUTPUT_FORMAT_NONE;
        auto format_name = part;
        if (format_name == "ubx") {
            change = OUTPUT_FORMAT_UBX;
        } else if (format_name == "nmea") {
            change = OUTPUT_FORMAT_NMEA;
        } else if (format_name == "rtcm") {
            change = OUTPUT_FORMAT_RTCM;
        } else if (format_name == "lpp-xer") {
            change = OUTPUT_FORMAT_LPP_XER;
        } else if (format_name == "lpp-uper") {
            change = OUTPUT_FORMAT_LPP_UPER;
        } else if (format_name == "lpp-rtcm-frame") {
            change = OUTPUT_FORMAT_LPP_RTCM_FRAME;
        } else if (format_name == "spartn") {
            change = OUTPUT_FORMAT_SPARTN;
        } else {
            throw std::invalid_argument("invalid output format name: \"" + format_name + "\"");
        }

        format |= change;
    }

    return format;
}

static OutputOption
parse_output_stdout(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_output_format(options);
    auto output = std::unique_ptr<io::Output>(new io::StdoutOutput());
    return {format, std::move(output)};
}

static OutputOption parse_output_file(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_output_format(options);
    if (options.find("path") == options.end()) {
        throw args::RequiredError("--output file requires 'path'");
    }

    auto path   = options.at("path");
    auto output = std::unique_ptr<io::Output>(new io::FileOutput(path, true, false, true));
    return {format, std::move(output)};
}

static io::BaudRate parse_output_baudrate(std::string const& str) {
    long baud_rate = 0;
    try {
        baud_rate = std::stol(str);
    } catch (...) {
        throw args::ParseError("--output serial 'baudrate' must be an integer");
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
    throw args::ParseError("--output serial 'baudrate' must be a valid baud rate");
}

static io::DataBits parse_output_databits(std::string const& str) {
    long databits = 0;
    try {
        databits = std::stol(str);
    } catch (...) {
        throw args::ParseError("--output serial 'data' must be an integer");
    }

    if (databits == 5) return io::DataBits::FIVE;
    if (databits == 6) return io::DataBits::SIX;
    if (databits == 7) return io::DataBits::SEVEN;
    if (databits == 8) return io::DataBits::EIGHT;
    throw args::ParseError("--output serial 'data' must be 5, 6, 7, or 8");
}

static io::StopBits parse_output_stopbits(std::string const& str) {
    long stopbits = 0;
    try {
        stopbits = std::stol(str);
    } catch (...) {
        throw args::ParseError("--output serial 'stop' must be an integer");
    }

    if (stopbits == 1) return io::StopBits::ONE;
    if (stopbits == 2) return io::StopBits::TWO;
    throw args::ParseError("--output serial 'stop' must be 1 or 2");
}

static io::ParityBit parse_output_paritybit(std::string const& str) {
    if (str == "none") return io::ParityBit::NONE;
    if (str == "odd") return io::ParityBit::ODD;
    if (str == "even") return io::ParityBit::EVEN;
    throw args::ParseError("--output serial 'parity' must be none, odd, or even");
}

static OutputOption
parse_output_serial(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_output_format(options);
    if (options.find("device") == options.end()) {
        throw args::RequiredError("--output serial requires 'device'");
    }

    auto baud_rate  = io::BaudRate::BR115200;
    auto data_bits  = io::DataBits::EIGHT;
    auto stop_bits  = io::StopBits::ONE;
    auto parity_bit = io::ParityBit::NONE;

    auto baud_rate_it = options.find("baudrate");
    if (baud_rate_it != options.end()) baud_rate = parse_output_baudrate(baud_rate_it->second);

    auto data_bits_it = options.find("data");
    if (data_bits_it != options.end()) data_bits = parse_output_databits(data_bits_it->second);

    auto stop_bits_it = options.find("stop");
    if (stop_bits_it != options.end()) stop_bits = parse_output_stopbits(stop_bits_it->second);

    auto parity_bit_it = options.find("parity");
    if (parity_bit_it != options.end()) parity_bit = parse_output_paritybit(parity_bit_it->second);

    auto device = options.at("device");
    auto output = std::unique_ptr<io::Output>(
        new io::SerialOutput(device, baud_rate, data_bits, stop_bits, parity_bit));
    return {format, std::move(output)};
}

static OutputOption parse_output(std::string const& source) {
    std::unordered_map<std::string, std::string> options;

    auto parts = split(source, ':');
    if (parts.size() == 1) {
        // No options
    } else if (parts.size() == 2) {
        auto args = split(parts[1], ',');
        for (std::string const& arg : args) {
            auto kv = split(arg, '=');
            if (kv.size() != 2) {
                throw args::ParseError("--output argument not in key=value format: \"" + arg +
                                       "\"");
            }
            options[kv[0]] = kv[1];
        }
    } else {
        throw args::ParseError("--output not in type:arguments format: \"" + source + "\"");
    }

    auto type = parts[0];
    if (type == "stdout") {
        return parse_output_stdout(options);
    } else if (type == "file") {
        return parse_output_file(options);
    } else if (type == "serial") {
        return parse_output_serial(options);
    } else {
        throw args::ParseError("--output type not recognized: \"" + type + "\"");
    }
}

static void parse_output_options(OutputOptions& output_options) {
    for (auto const& output : gOutputArgs.Get()) {
        output_options.outputs.push_back(parse_output(output));
    }
}

//
//
//

static InputFormat parse_input_format(std::unordered_map<std::string, std::string> const& options) {
    if (options.find("format") == options.end()) {
        return INPUT_FORMAT_ALL;
    }

    auto fmt    = options.at("format");
    auto parts  = split_at_any(fmt, "-+");
    auto format = INPUT_FORMAT_ALL;
    for (auto const& part : parts) {
        if (part.empty()) {
            continue;
        } else if (part.length() < 2) {
            throw args::ParseError("invalid input format format: \"" + part + "\"");
        }

        auto add_or_remove = true;
        if (part[0] == '+') {
            add_or_remove = true;
        } else if (part[0] == '-') {
            add_or_remove = false;
        } else {
            throw std::invalid_argument("expected '+' or '-'");
        }

        auto change      = INPUT_FORMAT_NONE;
        auto format_name = part.substr(1);
        if (format_name == "ubx") {
            change = INPUT_FORMAT_UBX;
        } else if (format_name == "nmea") {
            change = INPUT_FORMAT_NMEA;
        } else if (format_name == "rtcm") {
            change = INPUT_FORMAT_RTCM;
        } else if (format_name == "control") {
            change = INPUT_FORMAT_CTRL;
        } else if (format_name == "lpp") {
            change = INPUT_FORMAT_LPP;
        } else if (format_name == "all") {
            change = INPUT_FORMAT_ALL;
        } else {
            throw std::invalid_argument("invalid input format name: \"" + format_name + "\"");
        }

        if (add_or_remove) {
            format |= change;
        } else {
            format &= ~change;
        }
    }

    return format;
}

static InputOption parse_input_stdin(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_input_format(options);
    auto input  = std::unique_ptr<io::Input>(new io::StdinInput());
    return {format, std::move(input)};
}

static InputOption parse_input_file(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_input_format(options);
    if (options.find("path") == options.end()) {
        throw args::RequiredError("--input file requires 'path'");
    }

    auto path = options.at("path");
    auto bps  = 128 * 10;
    if (options.find("bps") != options.end()) {
        try {
            bps = std::stoi(options.at("bps"));
        } catch (...) {
            throw args::ParseError("--input file 'bps' must be an integer");
        }
    }

    auto tick_interval  = std::chrono::milliseconds(100);
    auto bytes_per_tick = static_cast<size_t>((bps + 9) / 10);

    auto input = std::unique_ptr<io::Input>(new io::FileInput(path, bytes_per_tick, tick_interval));
    return {format, std::move(input)};
}

static io::BaudRate parse_baudrate(std::string const& str) {
    long baud_rate = 0;
    try {
        baud_rate = std::stol(str);
    } catch (...) {
        throw args::ParseError("--input serial 'baudrate' must be an integer");
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
    throw args::ParseError("--input serial 'baudrate' must be a valid baud rate");
}

static io::DataBits parse_databits(std::string const& str) {
    long databits = 0;
    try {
        databits = std::stol(str);
    } catch (...) {
        throw args::ParseError("--input serial 'data' must be an integer");
    }

    if (databits == 5) return io::DataBits::FIVE;
    if (databits == 6) return io::DataBits::SIX;
    if (databits == 7) return io::DataBits::SEVEN;
    if (databits == 8) return io::DataBits::EIGHT;
    throw args::ParseError("--input serial 'data' must be 5, 6, 7, or 8");
}

static io::StopBits parse_stopbits(std::string const& str) {
    long stopbits = 0;
    try {
        stopbits = std::stol(str);
    } catch (...) {
        throw args::ParseError("--input serial 'stop' must be an integer");
    }

    if (stopbits == 1) return io::StopBits::ONE;
    if (stopbits == 2) return io::StopBits::TWO;
    throw args::ParseError("--input serial 'stop' must be 1 or 2");
}

static io::ParityBit parse_paritybit(std::string const& str) {
    if (str == "none") return io::ParityBit::NONE;
    if (str == "odd") return io::ParityBit::ODD;
    if (str == "even") return io::ParityBit::EVEN;
    throw args::ParseError("--input serial 'parity' must be none, odd, or even");
}

static InputOption parse_input_serial(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_input_format(options);
    if (options.find("device") == options.end()) {
        throw args::RequiredError("--input serial requires 'device'");
    }

    auto device = options.at("device");

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

    auto input = std::unique_ptr<io::Input>(
        new io::SerialInput(device, baud_rate, data_bits, stop_bits, parity_bit));
    return {format, std::move(input)};
}

static InputOption
parse_input_tcp_server(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_input_format(options);
    if (options.find("port") == options.end()) {
        throw args::RequiredError("--input tcp-server requires 'port'");
    }

    auto port = 0;
    try {
        port = std::stoi(options.at("port"));
    } catch (...) {
        throw args::ParseError("--input tcp-server 'port' must be an integer");
    }

    if (port < 0 || port > 65535) {
        throw args::ParseError("--input tcp-server 'port' must be in the range [0, 65535]");
    }

    auto input =
        std::unique_ptr<io::Input>(new io::TcpServerInput("0.0.0.0", static_cast<uint16_t>(port)));
    return {format, std::move(input)};
}

static InputOption
parse_input_tcp_client(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_input_format(options);
    if (options.find("host") == options.end()) {
        throw args::RequiredError("--input tcp-client requires 'host'");
    }
    if (options.find("port") == options.end()) {
        throw args::RequiredError("--input tcp-client requires 'port'");
    }

    auto host = options.at("host");
    auto port = 0;
    try {
        port = std::stoi(options.at("port"));
    } catch (...) {
        throw args::ParseError("--input tcp-client 'port' must be an integer");
    }

    if (port < 0 || port > 65535) {
        throw args::ParseError("--input tcp-client 'port' must be in the range [0, 65535]");
    }

    auto input =
        std::unique_ptr<io::Input>(new io::TcpClientInput(host, static_cast<uint16_t>(port)));
    return {format, std::move(input)};
}

static InputOption
parse_input_udp_server(std::unordered_map<std::string, std::string> const& options) {
    auto format = parse_input_format(options);
    if (options.find("port") == options.end()) {
        throw args::RequiredError("--input udp-server requires 'port'");
    }

    auto port = 0;
    try {
        port = std::stoi(options.at("port"));
    } catch (...) {
        throw args::ParseError("--input udp-server 'port' must be an integer");
    }

    if (port < 0 || port > 65535) {
        throw args::ParseError("--input udp-server 'port' must be in the range [0, 65535]");
    }

    auto input =
        std::unique_ptr<io::Input>(new io::UdpServerInput("0.0.0.0", static_cast<uint16_t>(port)));
    return {format, std::move(input)};
}

static InputOption parse_input(std::string const& source) {
    std::unordered_map<std::string, std::string> options;

    auto parts = split(source, ':');
    if (parts.size() == 1) {
        // No options
    } else if (parts.size() == 2) {
        auto args = split(parts[1], ',');
        for (std::string const& arg : args) {
            auto kv = split(arg, '=');
            if (kv.size() != 2) {
                throw args::ParseError("--input argument not in key=value format: \"" + arg + "\"");
            }
            options[kv[0]] = kv[1];
        }
    } else {
        throw args::ParseError("--input not in type:arguments format: \"" + source + "\"");
    }

    auto type = parts[0];
    if (type == "stdin") {
        return parse_input_stdin(options);
    } else if (type == "file") {
        return parse_input_file(options);
    } else if (type == "serial") {
        return parse_input_serial(options);
    } else if (type == "tcp-client") {
        return parse_input_tcp_client(options);
    } else if (type == "tcp-server") {
        return parse_input_tcp_server(options);
    } else if (type == "udp-server") {
        return parse_input_udp_server(options);
    } else if (type == "unix-socket") {
        throw args::ParseError("--input type not implemented: \"" + type + "\"");
    } else {
        throw args::ParseError("--input type not recognized: \"" + type + "\"");
    }
}

static void parse_input_options(InputOptions& input_options) {
    for (auto const& input : gInputArgs.Get()) {
        input_options.inputs.push_back(parse_input(input));
    }
}

//
// Location Information
//

static args::Group gLocationInformationGroup{
    "Location Infomation:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Flag gLiEnable{
    gLocationInformationGroup,
    "location-info",
    "Enable sending fake location information. Configure with '--fake-*' options.",
    {"fake-location-info", "fli"},
    args::Options::Single,
};
static args::Flag gLiForce{
    gLocationInformationGroup,
    "force-location-info",
    "Force Location Information (always send even if not requested)",
    {"force-location-info"},
    args::Options::Single,
};
static args::Flag gLiUnlocked{
    gLocationInformationGroup,
    "unlocked",
    "Send location reports without locking the update rate. By default, the update rate is locked "
    "to 1 second.",
    {"location-report-unlocked"},
    args::Options::Single,
};
static args::Flag gLiDisableNmeaLocation{
    gLocationInformationGroup,
    "disable-nmea-location",
    "Don't use location information from NMEA messages",
    {"disable-nmea-location"},
    args::Options::Single,
};
static args::Flag              gLiDisableUbxLocation{gLocationInformationGroup,
                                        "disable-ubx-location",
                                        "Don't use location information from UBX messages",
                                                     {"disable-ubx-location"},
                                        args::Options::Single};
static args::ValueFlag<int>    gLiUpdateRate{gLocationInformationGroup,
                                          "update-rate",
                                          "Update rate in milliseconds",
                                             {"update-rate"},
                                          args::Options::Single};
static args::ValueFlag<double> gLiLatitude{gLocationInformationGroup,
                                           "latitude",
                                           "Fake Latitude",
                                           {"fake-latitude", "flat"},
                                           args::Options::Single};
static args::ValueFlag<double> gLiLongitude{gLocationInformationGroup,
                                            "longitude",
                                            "Fake Longitude",
                                            {"fake-longitude", "flon"},
                                            args::Options::Single};
static args::ValueFlag<double> gLiAltitude{gLocationInformationGroup,
                                           "altitude",
                                           "Fake Altitude",
                                           {"fake-altitude", "falt"},
                                           args::Options::Single};
static args::Flag              gLiConf95to39{gLocationInformationGroup,
                                "confidence-95to39",
                                "Convert 95p confidence to 39p confidence",
                                             {"confidence-95to39"},
                                args::Options::Single};
static args::Flag              gLiConf95to68{
    gLocationInformationGroup,
    "confidence-95to68",
    "Rescale incoming semi-major/semi-minor axes from 95p to 68p confidence",
                 {"confidence-95to68"},
    args::Options::Single};
static args::Flag gLiOutputEllipse68{
    gLocationInformationGroup,
    "output-ellipse-68",
    "Output error ellipse with confidence 68p instead of 39p",
    {"output-ellipse-68"},
    args::Options::Single,
};
static args::ValueFlag<double> gLiOverrideHorizontalConfidence{
    gLocationInformationGroup,
    "override-horizontal-confidence",
    "Override horizontal confidence [0.0-1.0]",
    {"override-horizontal-confidence"},
    args::Options::Single,
};

//
// Control Options
//

static args::Group gControlGroup{
    "Control Options:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Group gControlInterface{
    gControlGroup,
    "Interface:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};

static args::Group gControlSerialGroup{
    gControlInterface,
    "Serial:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> gControlSerialDevice{
    gControlSerialGroup, "device", "Device", {"ctrl-serial"}, args::Options::Single};
static args::ValueFlag<int> gControlSerialBaudRate{
    gControlSerialGroup, "baud_rate", "Baud Rate", {"ctrl-serial-baud"}, args::Options::Single};
static args::ValueFlag<int> gControlSerialDataBits{
    gControlSerialGroup, "data_bits", "Data Bits", {"ctrl-serial-data"}, args::Options::Single};
static args::ValueFlag<int> gControlSerialStopBits{
    gControlSerialGroup, "stop_bits", "Stop Bits", {"ctrl-serial-stop"}, args::Options::Single};
static args::ValueFlag<std::string> gControlSerialParityBits{gControlSerialGroup,
                                                             "parity_bits",
                                                             "Parity Bits",
                                                             {"ctrl-serial-parity"},
                                                             args::Options::Single};

static args::Group gControlTcpGroup{
    gControlInterface,
    "TCP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> gControlTcpIpAddress{
    gControlTcpGroup, "ip_address", "Host or IP Address", {"ctrl-tcp"}, args::Options::Single};
static args::ValueFlag<uint16_t> gControlTcpPort{
    gControlTcpGroup, "port", "Port", {"ctrl-tcp-port"}, args::Options::Single};

static args::Group gControlUdpGroup{
    gControlInterface,
    "UDP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> gControlUdpIpAddress{
    gControlUdpGroup, "ip_address", "Host or IP Address", {"ctrl-udp"}, args::Options::Single};
static args::ValueFlag<uint16_t> gControlUdpPort{
    gControlUdpGroup, "port", "Port", {"ctrl-udp-port"}, args::Options::Single};

static args::Group gControlStdinGroup{
    gControlInterface,
    "Stdin:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::Flag gControlStdin{
    gControlStdinGroup, "stdin", "Stdin", {"ctrl-stdin"}, args::Options::Single};

static args::Group gControlUnixSocketGroup{
    gControlInterface,
    "Unix Socket:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};

static args::ValueFlag<std::string> gControlUnixSocketPath{
    gControlUnixSocketGroup, "path", "Path", {"ctrl-un"}, args::Options::Single};

//
// Options
//

static LocationServerOptions parse_location_server_options(Options& options) {
    LocationServerOptions location_server_options{};
    location_server_options.port                         = 5431;
    location_server_options.ssl                          = false;
    location_server_options.skip_connect                 = false;
    location_server_options.skip_request_assistance_data = false;

    if (gSkipConnect) {
        location_server_options.skip_connect = true;
    }

    if (gSkipRequestAssistanceData) {
        location_server_options.skip_request_assistance_data = true;
    }

    if (gLocationServerHost) {
        location_server_options.host = gLocationServerHost.Get();
    } else if (gLocationServerSlpHostImsi) {
        if (!options.identity_options.imsi) {
            throw args::RequiredError("`imsi` is required to use `slp-host-imsi`");
        } else if (options.identity_options.wait_for_identity) {
            throw args::ValidationError("`slp-host-imsi` cannot be used with `wait-for-identity`");
        }

        auto imsi   = *options.identity_options.imsi;
        auto digits = std::to_string(imsi).size();
        if (digits < 6) {
            throw args::ValidationError("`imsi` must be at least 6 digits long");
        }

        auto mcc = (imsi / static_cast<unsigned long long>(std::pow(10, digits - 3))) % 1000;
        auto mnc = (imsi / static_cast<unsigned long long>(std::pow(10, digits - 6))) % 1000;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "h-slp.%03llu.%03llu.pub.3gppnetwork.org", mnc, mcc);
        auto h_slp                   = std::string{buffer};
        location_server_options.host = h_slp;
    } else if (gLocationServerSlpHostCell) {
        auto mcc = options.cell_options.mcc;
        auto mnc = options.cell_options.mnc;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "h-slp.%03d.%03d.pub.3gppnetwork.org", mnc, mcc);
        auto h_slp                   = std::string{buffer};
        location_server_options.host = h_slp;
    } else {
        throw args::RequiredError("`host` or `slp-host-cell` or `slp-host-imsi` is required");
    }

    if (gLocationServerPort) {
        location_server_options.port = gLocationServerPort.Get();
    }

    if (gLocationServerSsl) {
        location_server_options.ssl = gLocationServerSsl.Get();
    }

    return location_server_options;
}

static IdentityOptions parse_identity_options() {
    IdentityOptions identity_options{};
    identity_options.use_supl_identity_fix = false;
    identity_options.wait_for_identity     = false;

    if (gMsisdn) {
        identity_options.msisdn =
            std::unique_ptr<unsigned long long>{new unsigned long long{gMsisdn.Get()}};
    }

    if (gImsi) {
        identity_options.imsi =
            std::unique_ptr<unsigned long long>{new unsigned long long{gImsi.Get()}};
    }

    if (gIpv4) {
        identity_options.ipv4 = std::unique_ptr<std::string>{new std::string{gIpv4.Get()}};
    }

    if (gWaitForIdentity) {
        identity_options.wait_for_identity = true;
    }

    if (!identity_options.wait_for_identity && !identity_options.msisdn && !identity_options.imsi &&
        !identity_options.ipv4) {
        identity_options.imsi =
            std::unique_ptr<unsigned long long>{new unsigned long long{2460813579lu}};
    }

    if (gUseSuplIdentityFix) {
        identity_options.use_supl_identity_fix = true;
    }

    return identity_options;
}

static CellOptions parse_cell_options() {
    CellOptions cell_options{};
    cell_options.mcc   = gMcc.Get();
    cell_options.mnc   = gMnc.Get();
    cell_options.tac   = gTac.Get();
    cell_options.cid   = gCi.Get();
    cell_options.is_nr = gIsNr ? gIsNr.Get() : false;
    return cell_options;
}

//
// Input
//

//
// u-blox
//

struct UbloxResult {
    std::unique_ptr<io::Input>  input;
    std::unique_ptr<io::Output> output;
};

static UbloxResult ublox_parse_serial() {
    assert(gUbloxSerialDevice);

    io::BaudRate baud_rate = io::BaudRate::BR115200;
    if (gUbloxSerialBaudRate) {
        auto baud_rate_value = gUbloxSerialBaudRate.Get();
        switch (baud_rate_value) {
        case 4800: baud_rate = io::BaudRate::BR4800; break;
        case 9600: baud_rate = io::BaudRate::BR9600; break;
        case 19200: baud_rate = io::BaudRate::BR19200; break;
        case 38400: baud_rate = io::BaudRate::BR38400; break;
        case 57600: baud_rate = io::BaudRate::BR57600; break;
        case 115200: baud_rate = io::BaudRate::BR115200; break;
        default: throw args::ValidationError("--ublox-serial-baud: invalid baud rate");
        }
    }

    auto data_bits = io::DataBits::EIGHT;
    if (gUbloxSerialDataBits) {
        switch (gUbloxSerialDataBits.Get()) {
        case 5: data_bits = io::DataBits::FIVE; break;
        case 6: data_bits = io::DataBits::SIX; break;
        case 7: data_bits = io::DataBits::SEVEN; break;
        case 8: data_bits = io::DataBits::EIGHT; break;
        default: throw args::ValidationError("--ublox-serial-data: invalid data bits");
        }
    }

    auto stop_bits = io::StopBits::ONE;
    if (gUbloxSerialStopBits) {
        switch (gUbloxSerialStopBits.Get()) {
        case 1: stop_bits = io::StopBits::ONE; break;
        case 2: stop_bits = io::StopBits::TWO; break;
        default: throw args::ValidationError("--ublox-serial-stop: invalid stop bits");
        }
    }

    auto parity_bit = io::ParityBit::NONE;
    if (gUbloxSerialParityBits) {
        if (gUbloxSerialParityBits.Get() == "none") {
            parity_bit = io::ParityBit::NONE;
        } else if (gUbloxSerialParityBits.Get() == "odd") {
            parity_bit = io::ParityBit::ODD;
        } else if (gUbloxSerialParityBits.Get() == "even") {
            parity_bit = io::ParityBit::EVEN;
        } else {
            throw args::ValidationError("--ublox-serial-parity: invalid parity");
        }
    }

    std::unique_ptr<io::Input> input{};
    input = std::unique_ptr<io::Input>(
        new io::SerialInput(gUbloxSerialDevice.Get(), baud_rate, data_bits, stop_bits, parity_bit));

    std::unique_ptr<io::Output> output{};
    if (!gReceiverReadonly) {
        output = std::unique_ptr<io::Output>(new io::SerialOutput(
            gUbloxSerialDevice.Get(), baud_rate, data_bits, stop_bits, parity_bit));
    }

    return UbloxResult{std::move(input), std::move(output)};
}

static UbloxResult ublox_parse_tcp() {
    assert(gUbloxTcpIpAddress);

    if (!gUbloxTcpPort) {
        throw args::RequiredError("ublox-tcp-port");
    }

    auto input = std::unique_ptr<io::Input>(
        new io::TcpClientInput(gUbloxTcpIpAddress.Get(), gUbloxTcpPort.Get()));
    return UbloxResult{std::move(input), nullptr};
}

static UbloxResult ublox_parse_interface() {
    if (gUbloxSerialDevice) {
        return ublox_parse_serial();
    } else if (gUbloxTcpIpAddress) {
        return ublox_parse_tcp();
    } else {
        throw args::RequiredError("No device/interface specified for u-blox receiver");
    }
}

#if 0
static Port ublox_parse_port() {
    if (ublox_receiver_port) {
        if (ublox_receiver_port.Get() == "uart1") {
            return Port::UART1;
        } else if (ublox_receiver_port.Get() == "uart2") {
            return Port::UART2;
        } else if (ublox_receiver_port.Get() == "i2c") {
            return Port::I2C;
        } else if (ublox_receiver_port.Get() == "usb") {
            return Port::USB;
        } else {
            throw args::ValidationError("Invalid port");
        }
    } else {
        if (ublox_serial_device) {
            return Port::UART1;
        } else if (ublox_i2c_device) {
            return Port::I2C;
        } else {
            throw args::RequiredError("u-blox port must be specified for this device/interface");
        }
    }
}
#endif

static bool print_receiver_options_parse() {
    if (gPrintMessages) {
        return true;
    } else {
        return false;
    }
}

static void ublox_parse_options(InputOptions& input_options, OutputOptions& output_options) {
    if (gUbloxSerialDevice || gUbloxTcpIpAddress) {
        auto interface = ublox_parse_interface();
        auto prm       = print_receiver_options_parse();
        input_options.print_ubx |= prm;
        if (interface.input) {
            input_options.inputs.emplace_back(InputOption{
                INPUT_FORMAT_UBX,
                std::move(interface.input),
            });
        }
        if (interface.output) {
            output_options.outputs.emplace_back(OutputOption{
                OUTPUT_FORMAT_RTCM | OUTPUT_FORMAT_SPARTN,
                std::move(interface.output),
            });
        }
    }
}

struct NmeaResult {
    std::unique_ptr<io::Input>  input;
    std::unique_ptr<io::Output> output;
};

static NmeaResult nmea_parse_serial() {
    io::BaudRate baud_rate = io::BaudRate::BR115200;
    if (gNmeaSerialBaudRate) {
        auto baud_rate_value = gNmeaSerialBaudRate.Get();
        switch (baud_rate_value) {
        case 4800: baud_rate = io::BaudRate::BR4800; break;
        case 9600: baud_rate = io::BaudRate::BR9600; break;
        case 19200: baud_rate = io::BaudRate::BR19200; break;
        case 38400: baud_rate = io::BaudRate::BR38400; break;
        case 57600: baud_rate = io::BaudRate::BR57600; break;
        case 115200: baud_rate = io::BaudRate::BR115200; break;
        default: throw args::ValidationError("--nmea-serial-baud: invalid baud rate");
        }
    }

    auto data_bits = io::DataBits::EIGHT;
    if (gNmeaSerialDataBits) {
        switch (gNmeaSerialDataBits.Get()) {
        case 5: data_bits = io::DataBits::FIVE; break;
        case 6: data_bits = io::DataBits::SIX; break;
        case 7: data_bits = io::DataBits::SEVEN; break;
        case 8: data_bits = io::DataBits::EIGHT; break;
        default: throw args::ValidationError("--nmea-serial-data: invalid data bits");
        }
    }

    auto stop_bits = io::StopBits::ONE;
    if (gNmeaSerialStopBits) {
        switch (gNmeaSerialStopBits.Get()) {
        case 1: stop_bits = io::StopBits::ONE; break;
        case 2: stop_bits = io::StopBits::TWO; break;
        default: throw args::ValidationError("--nmea-serial-stop: invalid stop bits");
        }
    }

    auto parity_bit = io::ParityBit::NONE;
    if (gNmeaSerialParityBits) {
        if (gNmeaSerialParityBits.Get() == "none") {
            parity_bit = io::ParityBit::NONE;
        } else if (gNmeaSerialParityBits.Get() == "odd") {
            parity_bit = io::ParityBit::ODD;
        } else if (gNmeaSerialParityBits.Get() == "even") {
            parity_bit = io::ParityBit::EVEN;
        } else {
            throw args::ValidationError("--nmea-serial-parity: invalid parity");
        }
    }

    std::unique_ptr<io::Input> input{};
    input = std::unique_ptr<io::Input>(
        new io::SerialInput(gNmeaSerialDevice.Get(), baud_rate, data_bits, stop_bits, parity_bit));

    std::unique_ptr<io::Output> output{};
    if (!gReceiverReadonly) {
        output = std::unique_ptr<io::Output>(new io::SerialOutput(
            gNmeaSerialDevice.Get(), baud_rate, data_bits, stop_bits, parity_bit));
    }
    return NmeaResult{std::move(input), std::move(output)};
}

static NmeaResult nmea_parse_tcp_client() {
    assert(gNmeaTcpIpAddress);

    if (!gNmeaTcpPort) {
        throw args::RequiredError("nmea-tcp-port");
    }

    auto input = std::unique_ptr<io::Input>(
        new io::TcpClientInput(gNmeaTcpIpAddress.Get(), gNmeaTcpPort.Get()));
    return NmeaResult{std::move(input), nullptr};
}

static NmeaResult nmea_parse_interface() {
    if (gNmeaSerialDevice) {
        return nmea_parse_serial();
    } else if (gNmeaTcpIpAddress) {
        return nmea_parse_tcp_client();
    } else {
        throw args::RequiredError("No device/interface specified for NMEA receiver");
    }
}

static void nmea_parse_options(InputOptions& input_options, OutputOptions& output_options) {
    if (gNmeaSerialDevice || gNmeaTcpIpAddress) {
#if 0
        // TODO(ewasjon): Add support for NMEA export
        if (nmea_export_un) {
            auto interface = interface::Interface::unix_socket_stream(nmea_export_un.Get(), true);
            interface->open();
            output_options.outputs.emplace_back(OutputOption{
                OUTPUT_FORMAT_NMEA,
                std::unique_ptr<Interface>(interface),
            });
        }

        if (nmea_export_tcp) {
            if (!nmea_export_tcp_port) {
                throw args::RequiredError("nmea-export-tcp-port");
            }

            auto interface =
                interface::Interface::tcp(nmea_export_tcp.Get(), nmea_export_tcp_port.Get(), true);
            interface->open();
            output_options.outputs.emplace_back(OutputOption{
                OUTPUT_FORMAT_NMEA,
                std::unique_ptr<Interface>(interface),
            });
        }
#endif

        auto interface = nmea_parse_interface();
        auto prm       = print_receiver_options_parse();
        input_options.print_nmea |= prm;
        if (interface.input) {
            input_options.inputs.emplace_back(InputOption{
                INPUT_FORMAT_NMEA,
                std::move(interface.input),
            });
        }
        if (interface.output) {
            output_options.outputs.emplace_back(OutputOption{
                OUTPUT_FORMAT_RTCM | OUTPUT_FORMAT_SPARTN,
                std::move(interface.output),
            });
        }
    }
}

static LocationInformationOptions parse_location_information_options() {
    LocationInformationOptions location_information{};
    location_information.fake_location_info             = false;
    location_information.latitude                       = 69.0599730655754;
    location_information.longitude                      = 20.54864403253676;
    location_information.altitude                       = 0;
    location_information.force                          = false;
    location_information.unlock_update_rate             = false;
    location_information.update_rate                    = 1000;
    location_information.convert_confidence_95_to_39    = false;
    location_information.convert_confidence_95_to_68    = false;
    location_information.output_ellipse_68              = false;
    location_information.override_horizontal_confidence = -1.0;
    location_information.disable_nmea_location          = false;
    location_information.disable_ubx_location           = false;

    if (gLiForce) {
        location_information.force = true;
    }

    if (gLiUnlocked) {
        location_information.unlock_update_rate = true;
    }

    if (gLiUpdateRate) {
        location_information.update_rate = gLiUpdateRate.Get();
        if (location_information.update_rate < 10) {
            throw args::ValidationError("Update rate cannot be less than 10 milliseconds");
        }
    }

    if (gLiDisableNmeaLocation) {
        location_information.disable_nmea_location = true;
    }

    if (gLiDisableUbxLocation) {
        location_information.disable_ubx_location = true;
    }

    if (gLiEnable) {
        location_information.fake_location_info = true;
        if (gLiLatitude) {
            location_information.latitude = gLiLatitude.Get();
        }

        if (gLiLongitude) {
            location_information.longitude = gLiLongitude.Get();
        }

        if (gLiAltitude) {
            location_information.altitude = gLiAltitude.Get();
        }
    }

    if (gLiConf95to39 || gLiConf95to68) {
        location_information.convert_confidence_95_to_39 = true;
    }

    if (gLiOutputEllipse68) {
        location_information.output_ellipse_68 = true;
    }

    if (gLiOverrideHorizontalConfidence) {
        location_information.override_horizontal_confidence = gLiOverrideHorizontalConfidence.Get();
        if (location_information.override_horizontal_confidence < 0 ||
            location_information.override_horizontal_confidence > 1) {
            throw args::ValidationError(
                "Override horizontal confidence must be between 0.0 and 1.0");
        }
    }

    return location_information;
}

static std::unique_ptr<io::Input> control_parse_serial() {
    assert(gControlSerialDevice);

    io::BaudRate baud_rate = io::BaudRate::BR115200;
    if (gControlSerialBaudRate) {
        auto baud_rate_value = gControlSerialBaudRate.Get();
        switch (baud_rate_value) {
        case 4800: baud_rate = io::BaudRate::BR4800; break;
        case 9600: baud_rate = io::BaudRate::BR9600; break;
        case 19200: baud_rate = io::BaudRate::BR19200; break;
        case 38400: baud_rate = io::BaudRate::BR38400; break;
        case 57600: baud_rate = io::BaudRate::BR57600; break;
        case 115200: baud_rate = io::BaudRate::BR115200; break;
        default: throw args::ValidationError("--ctrl-serial-baud: invalid baud rate");
        }
    }

    auto data_bits = io::DataBits::EIGHT;
    if (gControlSerialDataBits) {
        switch (gControlSerialDataBits.Get()) {
        case 5: data_bits = io::DataBits::FIVE; break;
        case 6: data_bits = io::DataBits::SIX; break;
        case 7: data_bits = io::DataBits::SEVEN; break;
        case 8: data_bits = io::DataBits::EIGHT; break;
        default: throw args::ValidationError("--ctrl-serial-data: invalid data bits");
        }
    }

    auto stop_bits = io::StopBits::ONE;
    if (gControlSerialStopBits) {
        switch (gControlSerialStopBits.Get()) {
        case 1: stop_bits = io::StopBits::ONE; break;
        case 2: stop_bits = io::StopBits::TWO; break;
        default: throw args::ValidationError("--ctrl-serial-stop: invalid stop bits");
        }
    }

    auto parity_bit = io::ParityBit::NONE;
    if (gControlSerialParityBits) {
        if (gControlSerialParityBits.Get() == "none") {
            parity_bit = io::ParityBit::NONE;
        } else if (gControlSerialParityBits.Get() == "odd") {
            parity_bit = io::ParityBit::ODD;
        } else if (gControlSerialParityBits.Get() == "even") {
            parity_bit = io::ParityBit::EVEN;
        } else {
            throw args::ValidationError("--ctrl-serial-parity: invalid parity");
        }
    }

    return std::unique_ptr<io::Input>(new io::SerialInput(gControlSerialDevice.Get(), baud_rate,
                                                          data_bits, stop_bits, parity_bit));
}

static std::unique_ptr<io::Input> control_parse_tcp() {
    assert(gControlTcpIpAddress);

    if (!gControlTcpPort) {
        throw args::RequiredError("ctrl-tcp-port");
    }

    return std::unique_ptr<io::Input>(
        new io::TcpClientInput(gControlTcpIpAddress.Get(), gControlTcpPort.Get()));
}

static std::unique_ptr<io::Input> control_parse_stdin() {
    return std::unique_ptr<io::Input>(new io::StdinInput());
}

static std::unique_ptr<io::Input> control_parse_interface() {
    if (gControlSerialDevice) {
        return control_parse_serial();
    } else if (gControlTcpIpAddress) {
        return control_parse_tcp();
    } else if (gControlStdin) {
        return control_parse_stdin();
    } else if (gControlUnixSocketPath) {
        __builtin_unreachable();
    } else {
        throw args::RequiredError("No device/interface specified for control interface");
    }
}

static void parse_control_options(InputOptions& input_options) {
    if (gControlSerialDevice || gControlTcpIpAddress || gControlStdin || gControlUnixSocketPath) {
        auto input = control_parse_interface();
        input_options.inputs.emplace_back(InputOption{
            INPUT_FORMAT_CTRL,
            std::move(input),
        });
    }
}

//
//
//

static args::Group gLogGroup{
    "Log:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Flag gLogDebug{
    gLogGroup, "debug", "Set log level to debug", {"debug"}, args::Options::Single};
static args::Flag gLogVerbose{
    gLogGroup, "verbose", "Set log level to verbose", {"verbose"}, args::Options::Single};
static args::Flag gLogInfo{
    gLogGroup, "info", "Set log level to info", {"info"}, args::Options::Single};
static args::Flag gLogWarning{
    gLogGroup, "warning", "Set log level to warning", {"warning"}, args::Options::Single};
static args::Flag gLogError{
    gLogGroup, "error", "Set log level to error", {"error"}, args::Options::Single};
static args::ValueFlagList<std::string> gLogModules{
    gLogGroup, "module", "<module>=<level>", {"lm"}};

static void parse_log_level(Options& config) {
    config.log_level = loglet::Level::Info;

    if (gLogDebug) {
        config.log_level = loglet::Level::Debug;
    } else if (gLogVerbose) {
        config.log_level = loglet::Level::Verbose;
    } else if (gLogInfo) {
        config.log_level = loglet::Level::Info;
    } else if (gLogWarning) {
        config.log_level = loglet::Level::Warning;
    } else if (gLogError) {
        config.log_level = loglet::Level::Error;
    }

    for (auto const& module : gLogModules) {
        auto parts = split(module, '=');
        if (parts.size() != 2) {
            throw args::ValidationError("Invalid log module: " + module);
        }

        auto level = loglet::Level::Disabled;
        if (parts[1] == "verbose") {
            level = loglet::Level::Verbose;
        } else if (parts[1] == "debug") {
            level = loglet::Level::Debug;
        } else if (parts[1] == "info") {
            level = loglet::Level::Info;
        } else if (parts[1] == "warning") {
            level = loglet::Level::Warning;
        } else if (parts[1] == "error") {
            level = loglet::Level::Error;
        } else if (parts[1] == "disable") {
            level = loglet::Level::Disabled;
        } else {
            throw args::ValidationError("Invalid log level: " + parts[1]);
        }

        config.module_levels[parts[0]] = level;
    }
}

//
// Option Parser
//

OptionParser::OptionParser() {}

void OptionParser::add_command(Command* command) {
    add_command(std::unique_ptr<Command>(command));
}

void OptionParser::add_command(std::unique_ptr<Command> command) {
    mCommands.push_back(std::move(command));
}

int OptionParser::parse_and_execute(int argc, char** argv) {
    args::ArgumentParser parser(
        "3GPP LPP Example (" CLIENT_VERSION
        ") - This sample code is a simple client that asks for assistance data from a location "
        "server. It can handle OSR, SSR, and AGNSS requests. The assistance data can converted to "
        "RTCM or SPARTN before being sent to a GNSS receiver or other interface. The client also "
        "supports to 3GPP LPP Provide Location Information, which can be used to send the device's "
        "location to the location server.");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    // Commands
    args::Group commands{
        parser,
        "Commands:",
        args::Group::Validators::DontCare,
        args::Options::Global,
    };

    std::vector<std::unique_ptr<args::Command>> args_commands;
    for (auto& command : mCommands) {
        auto command_ptr = command.get();
        args_commands.emplace_back(new args::Command(
            commands, command->name(), command->description(),
            [command_ptr](args::Subparser& subparser) {
                command_ptr->parse(subparser);
                subparser.Parse();

                Options options{};
                options.cell_options                 = parse_cell_options();
                options.identity_options             = parse_identity_options();
                options.location_server_options      = parse_location_server_options(options);
                options.location_information_options = parse_location_information_options();
                parse_control_options(options.input_options);
                parse_log_level(options);

                parse_output_options(options.output_options);
                parse_input_options(options.input_options);
                ublox_parse_options(options.input_options, options.output_options);
                nmea_parse_options(options.input_options, options.output_options);

                command_ptr->execute(std::move(options));
            }));
    }

    // Defaults
    gUbloxSerialBaudRate.HelpDefault("115200");
    gUbloxSerialDataBits.HelpDefault("8");
    gUbloxSerialDataBits.HelpChoices({"5", "6", "7", "8"});
    gUbloxSerialStopBits.HelpDefault("1");
    gUbloxSerialStopBits.HelpChoices({"1", "2"});
    gUbloxSerialParityBits.HelpDefault("none");
    gUbloxSerialParityBits.HelpChoices({
        "none",
        "odd",
        "even",
    });

    gNmeaSerialBaudRate.HelpDefault("115200");
    gNmeaSerialDataBits.HelpDefault("8");
    gNmeaSerialDataBits.HelpChoices({"5", "6", "7", "8"});
    gNmeaSerialStopBits.HelpDefault("1");
    gNmeaSerialStopBits.HelpChoices({"1", "2"});
    gNmeaSerialParityBits.HelpDefault("none");
    gNmeaSerialParityBits.HelpChoices({
        "none",
        "odd",
        "even",
    });

    gLocationServerPort.HelpDefault("5431");
    gLocationServerSsl.HelpDefault("false");
    gImsi.HelpDefault("2460813579");

    gLiLatitude.HelpDefault("69.0599730655754");
    gLiLongitude.HelpDefault("20.54864403253676");
    gLiAltitude.HelpDefault("0");

    gControlSerialBaudRate.HelpDefault("115200");
    gControlSerialDataBits.HelpDefault("8");
    gControlSerialDataBits.HelpChoices({"5", "6", "7", "8"});
    gControlSerialStopBits.HelpDefault("1");
    gControlSerialStopBits.HelpChoices({"1", "2"});
    gControlSerialParityBits.HelpDefault("none");
    gControlSerialParityBits.HelpChoices({
        "none",
        "odd",
        "even",
    });

    // Globals
    args::GlobalOptions location_server_globals{parser, gLocationServer};
    args::GlobalOptions identity_globals{parser, gIdentity};
    args::GlobalOptions cell_information_globals{parser, gCellInformation};
    args::GlobalOptions ublox_receiver_globals{parser, gUbloxReceiverGroup};
    args::GlobalOptions nmea_receiver_globals{parser, gNmeaReceiverGroup};
    args::GlobalOptions other_receiver_globals{parser, gOtherReceiverGroup};
    args::GlobalOptions output_globals{parser, gOutputGroup};
    args::GlobalOptions input_globals{parser, gInputGroup};
    args::GlobalOptions location_information_globals{parser, gLocationInformationGroup};
    args::GlobalOptions control_options_globals{parser, gControlGroup};
    args::GlobalOptions log_globals{parser, gLogGroup};

    // Parse
    try {
        parser.ParseCLI(argc, argv);
        if (version) {
            std::cout << "3GPP LPP Example (" << CLIENT_VERSION << ")" << std::endl;
            return 0;
        }

        return 0;
    } catch (args::ValidationError const& e) {
        std::cerr << e.what() << std::endl;
        parser.Help(std::cerr);
        return 1;
    } catch (args::Help const&) {
        std::cout << parser;
        return 0;
    } catch (args::ParseError const& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
