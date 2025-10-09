#include "../config.hpp"
#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(client, config)

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

namespace print {

static args::Group gGroup{"Print:"};
static args::ValueFlagList<std::string> gArgs{
    gGroup,
    "print",
    "Add a print inspector.\n"
    "Usage: --print <format>[,<arguments>]\n\n"
    "Arguments:\n"
    "  itags=<tag>[+<tag>...]\n"
    "  xtags=<tag>[+<tag>...]\n\n"
    "Formats:\n"
    "  ubx, nmea, rtcm, ctrl\n"
    "Examples:\n"
    "  --print nmea\n"
    "  --print rtcm,itags=test\n"
    "  --print nmea,xtags=abc",
    {"print"},
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions globals{parser, gGroup};
}

static OutputFormat parse_format(std::string const& str) {
    if (str == "ubx") return OUTPUT_FORMAT_UBX;
    if (str == "nmea") return OUTPUT_FORMAT_NMEA;
    if (str == "rtcm") return OUTPUT_FORMAT_RTCM;
    if (str == "ctrl") return OUTPUT_FORMAT_CTRL;
    throw args::ValidationError("--print format: invalid format, got `" + str + "`");
}

static std::vector<std::string> parse_tags(std::string const& str) {
    auto parts = ::split(str, '+');
    return parts;
}

static std::vector<std::string>
parse_itags_from_options(std::unordered_map<std::string, std::string> const& options) {
    if (options.find("itags") == options.end()) {
        return {};
    } else {
        return parse_tags(options.at("itags"));
    }
}

static std::vector<std::string>
parse_xtags_from_options(std::unordered_map<std::string, std::string> const& options) {
    if (options.find("xtags") == options.end()) {
        return {};
    } else {
        return parse_tags(options.at("xtags"));
    }
}

static std::unordered_map<std::string, std::string> parse_options(std::string const& str) {
    auto                                         parts = ::split(str, ',');
    std::unordered_map<std::string, std::string> options;
    for (size_t i = 1; i < parts.size(); i++) {
        auto kv = ::split(parts[i], '=');
        if (kv.size() != 2) {
            throw args::ValidationError("--print: invalid option, got `" + parts[i] + "`");
        }
        options[kv[0]] = kv[1];
    }
    return options;
}

static PrintInterface parse_interface(std::string const& source) {
    auto parts = ::split(source, ',');
    if (parts.empty()) {
        throw args::ParseError("--print: empty format");
    }

    auto format  = parse_format(parts[0]);
    auto options = parse_options(source);
    auto itags   = parse_itags_from_options(options);
    auto xtags   = parse_xtags_from_options(options);

    return PrintInterface::create(format, std::move(itags), std::move(xtags));
}

void parse(Config* config) {
    for (auto const& print : gArgs.Get()) {
        config->print.prints.push_back(parse_interface(print));
    }

    for (auto& print : config->print.prints) {
        for (auto const& itag : print.include_tags) {
            config->register_tag(itag);
        }

        for (auto const& xtag : print.exclude_tags) {
            config->register_tag(xtag);
        }
    }
}

void dump(PrintConfig const& config) {
    for (auto const& print : config.prints) {
        std::stringstream itag_stream;
        for (size_t i = 0; i < print.include_tags.size(); i++) {
            if (i > 0) itag_stream << ",";
            itag_stream << print.include_tags[i];
        }
        auto itag_str = itag_stream.str();

        std::stringstream xtag_stream;
        for (size_t i = 0; i < print.exclude_tags.size(); i++) {
            if (i > 0) xtag_stream << ",";
            xtag_stream << print.exclude_tags[i];
        }
        auto xtag_str = xtag_stream.str();

        DEBUGF("print: %s%s%s%s | include=%s[%" PRIu64 "] | exclude=%s[%" PRIu64 "]",
               (print.format & OUTPUT_FORMAT_UBX) ? "ubx " : "",
               (print.format & OUTPUT_FORMAT_NMEA) ? "nmea " : "",
               (print.format & OUTPUT_FORMAT_RTCM) ? "rtcm " : "",
               (print.format & OUTPUT_FORMAT_CTRL) ? "ctrl " : "", itag_str.c_str(),
               print.include_tag_mask, xtag_str.c_str(), print.exclude_tag_mask);
    }
}

}  // namespace print
