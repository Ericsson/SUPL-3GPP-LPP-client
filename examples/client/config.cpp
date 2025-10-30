#include "config.hpp"
#include <cxx11_compat.hpp>
#include <io/file.hpp>
#include <io/serial.hpp>
#include <io/stdin.hpp>
#include <io/stdout.hpp>
#include <io/tcp.hpp>
#include <io/udp.hpp>
#include <loglet/loglet.hpp>
#include "processor/chunked_log.hpp"

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

LOGLET_MODULE2(client, config);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(client, config)

std::vector<std::string> split(std::string const& str, char delim) {
    std::vector<std::string> tokens;
    std::string              token;
    std::istringstream       token_stream(str);
    while (std::getline(token_stream, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

namespace config {

void dump(Config* config) {
    DEBUGF("config:");
    DEBUG_INDENT_SCOPE();

    {
        DEBUGF("location server:");
        DEBUG_INDENT_SCOPE();
        ::ls::dump(config->location_server);
    }

    {
        DEBUGF("agnss:");
        DEBUG_INDENT_SCOPE();
        ::agnss::dump(config->agnss);
    }

    {
        DEBUGF("identity:");
        DEBUG_INDENT_SCOPE();
        ::identity::dump(config->identity);
    }

    {
        DEBUGF("assistance data:");
        DEBUG_INDENT_SCOPE();
        ::ad::dump(config->assistance_data);
    }

    {
        DEBUGF("location information:");
        DEBUG_INDENT_SCOPE();
        ::li::dump(config->location_information);
    }

    {
        DEBUGF("input:");
        DEBUG_INDENT_SCOPE();
        ::input::dump(config->input);
    }

    {
        DEBUGF("output:");
        DEBUG_INDENT_SCOPE();
        ::output::dump(config->output);
    }

    {
        DEBUGF("print:");
        DEBUG_INDENT_SCOPE();
        ::print::dump(config->print);
    }

    {
        DEBUGF("gnss:");
        DEBUG_INDENT_SCOPE();
        ::gnss::dump(config->gnss);
    }

#ifdef INCLUDE_GENERATOR_RTCM
    {
        DEBUGF("lpp2rtcm:");
        DEBUG_INDENT_SCOPE();
        ::lpp2rtcm::dump(config->lpp2rtcm);
    }
    {
        DEBUGF("lpp2frame-rtcm:");
        DEBUG_INDENT_SCOPE();
        ::lpp2frame_rtcm::dump(config->lpp2frame_rtcm);
    }
#endif

#ifdef INCLUDE_GENERATOR_SPARTN
    {
        DEBUGF("lpp2spartn:");
        DEBUG_INDENT_SCOPE();
        ::lpp2spartn::dump(config->lpp2spartn);
    }
#endif

#ifdef INCLUDE_GENERATOR_TOKORO
    {
        DEBUGF("tokoro:");
        DEBUG_INDENT_SCOPE();
        ::tokoro::dump(config->tokoro);
    }
#endif

#ifdef INCLUDE_GENERATOR_IDOKEIDO
    {
        DEBUGF("idokeido:");
        DEBUG_INDENT_SCOPE();
        ::idokeido::dump(config->idokeido);
    }
#endif

    {
        DEBUGF("logging:");
        DEBUG_INDENT_SCOPE();
        ::logging::dump(config->logging);
    }

#ifdef DATA_TRACING
    {
        DEBUGF("data tracing:");
        DEBUG_INDENT_SCOPE();
        ::data_tracing::dump(config->data_tracing);
    }
#endif

    {
        DEBUGF("ubx config:");
        DEBUG_INDENT_SCOPE();
        ::ubx_config::dump(config->ubx_config);
    }
}

bool parse(int argc, char** argv, Config* config) {
    args::ArgumentParser parser("S3LC Client (" CLIENT_VERSION ")");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    ::ls::setup(parser);
    ::agnss::setup(parser);
    ::identity::setup(parser);
    ::ad::setup(parser);
    ::li::setup(parser);
    ::input::setup(parser);
    ::output::setup(parser);
    ::print::setup(parser);
    ::gnss::setup(parser);
#ifdef INCLUDE_GENERATOR_RTCM
    ::lpp2rtcm::setup(parser);
    ::lpp2frame_rtcm::setup(parser);
#endif
#ifdef INCLUDE_GENERATOR_SPARTN
    ::lpp2spartn::setup(parser);
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
    ::tokoro::setup(parser);
#endif
#ifdef INCLUDE_GENERATOR_IDOKEIDO
    ::idokeido::setup(parser);
#endif
    ::logging::setup(parser);
#ifdef DATA_TRACING
    ::data_tracing::setup(parser);
#endif
    ::ubx_config::setup(parser);

    config->next_tag_bit_mask = 1;
    config->tag_to_bit_mask.clear();

    // TODO(ewasjon): Use logger library here
    try {
        parser.ParseCLI(argc, argv);
        if (version) {
            std::cout << "S3LC Client (" << CLIENT_VERSION << ")" << std::endl;
            return true;
        }

        ::ls::parse(config);
        ::agnss::parse(config);
        ::identity::parse(config);
        ::ad::parse(config);
        ::li::parse(config);
        ::input::parse(config);
        ::output::parse(config);
        ::print::parse(config);
        ::gnss::parse(config);
#ifdef INCLUDE_GENERATOR_RTCM
        ::lpp2rtcm::parse(config);
        ::lpp2frame_rtcm::parse(config);
#endif
#ifdef INCLUDE_GENERATOR_SPARTN
        ::lpp2spartn::parse(config);
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
        ::tokoro::parse(config);
#endif
#ifdef INCLUDE_GENERATOR_IDOKEIDO
        ::idokeido::parse(config);
#endif
        ::logging::parse(config);
#ifdef DATA_TRACING
        ::data_tracing::parse(config);
#endif
        ::ubx_config::parse(config);

        config->assistance_data.gps &= config->gnss.gps;
        config->assistance_data.glonass &= config->gnss.glonass;
        config->assistance_data.galileo &= config->gnss.galileo;
        config->assistance_data.beidou &= config->gnss.beidou;

#ifdef INCLUDE_GENERATOR_RTCM
        config->lpp2rtcm.generate_gps &= config->gnss.gps;
        config->lpp2rtcm.generate_glonass &= config->gnss.glonass;
        config->lpp2rtcm.generate_galileo &= config->gnss.galileo;
        config->lpp2rtcm.generate_beidou &= config->gnss.beidou;
#endif

#ifdef INCLUDE_GENERATOR_SPARTN
        config->lpp2spartn.generate_gps &= config->gnss.gps;
        config->lpp2spartn.generate_glonass &= config->gnss.glonass;
        config->lpp2spartn.generate_galileo &= config->gnss.galileo;
        config->lpp2spartn.generate_beidou &= config->gnss.beidou;
#endif

#ifdef INCLUDE_GENERATOR_TOKORO
        config->tokoro.generate_gps &= config->gnss.gps;
        config->tokoro.generate_glonass &= config->gnss.glonass;
        config->tokoro.generate_galileo &= config->gnss.galileo;
        config->tokoro.generate_beidou &= config->gnss.beidou;
#endif

#ifdef INCLUDE_GENERATOR_IDOKEIDO
        config->idokeido.gps &= config->gnss.gps;
        config->idokeido.glonass &= config->gnss.glonass;
        config->idokeido.galileo &= config->gnss.galileo;
        config->idokeido.beidou &= config->gnss.beidou;
#endif

        return true;
    } catch (args::ValidationError const& e) {
        parser.Help(std::cerr);
        ERRORF("validation error: %s", e.what());
        return false;
    } catch (args::Help const&) {
        parser.Help(std::cerr);
        return false;
    } catch (args::ParseError const& e) {
        parser.Help(std::cerr);
        ERRORF("parse error: %s", e.what());
        return false;
    } catch (std::exception const& e) {
        ERRORF("unknown error: %s", e.what());
        return false;
    }
}

}  // namespace config
