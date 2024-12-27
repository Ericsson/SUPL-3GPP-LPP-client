#include "config.hpp"
#include <io/file.hpp>
#include <io/serial.hpp>
#include <io/stdin.hpp>
#include <io/stdout.hpp>
#include <io/tcp.hpp>
#include <io/udp.hpp>
#include <loglet/loglet.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-dtor"
#pragma GCC diagnostic ignored "-Wnewline-eof"
#pragma GCC diagnostic ignored "-Wmissing-variable-declarations"
#pragma GCC diagnostic ignored "-Winconsistent-missing-destructor-override"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wshadow-field"
#include <args.hpp>
#pragma GCC diagnostic pop

#define LOGLET_CURRENT_MODULE "config"

namespace config {

static std::vector<std::string> split(std::string const& str, char delim) {
    std::vector<std::string> tokens;
    std::string              token;
    std::istringstream       token_stream(str);
    while (std::getline(token_stream, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

#include "config/assistance_data.cpp"
#include "config/gnss.cpp"
#include "config/identity.cpp"
#include "config/input.cpp"
#include "config/location_information.cpp"
#include "config/location_server.cpp"
#include "config/logging.cpp"
#include "config/output.cpp"
#ifdef INCLUDE_GENERATOR_RTCM
#include "config/lpp2rtcm.cpp"
#endif
#ifdef INCLUDE_GENERATOR_SPARTN
#include "config/lpp2spartn.cpp"
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
#include "config/tokoro.cpp"
#endif
#ifdef DATA_TRACING
#include "config/data_tracing.cpp"
#endif

static void dump(Config* config) {
    DEBUGF("config:");
    LOGLET_DINDENT_SCOPE();

    {
        DEBUGF("location server:");
        LOGLET_DINDENT_SCOPE();
        ls::dump(config->location_server);
    }

    {
        DEBUGF("identity:");
        LOGLET_DINDENT_SCOPE();
        identity::dump(config->identity);
    }

    {
        DEBUGF("assistance data:");
        LOGLET_DINDENT_SCOPE();
        ad::dump(config->assistance_data);
    }

    {
        DEBUGF("location information:");
        LOGLET_DINDENT_SCOPE();
        li::dump(config->location_information);
    }

    {
        DEBUGF("input:");
        LOGLET_DINDENT_SCOPE();
        input::dump(config->input);
    }

    {
        DEBUGF("output:");
        LOGLET_DINDENT_SCOPE();
        output::dump(config->output);
    }

    {
        DEBUGF("gnss:");
        LOGLET_DINDENT_SCOPE();
        gnss::dump(config->gnss);
    }

#ifdef INCLUDE_GENERATOR_RTCM
    {
        DEBUGF("lpp2rtcm:");
        LOGLET_DINDENT_SCOPE();
        lpp2rtcm::dump(config->lpp2rtcm);
    }
#endif

#ifdef INCLUDE_GENERATOR_SPARTN
    {
        DEBUGF("lpp2spartn:");
        LOGLET_DINDENT_SCOPE();
        lpp2spartn::dump(config->lpp2spartn);
    }
#endif

#ifdef INCLUDE_GENERATOR_TOKORO
    {
        DEBUGF("tokoro:");
        LOGLET_DINDENT_SCOPE();
        tokoro::dump(config->tokoro);
    }
#endif

    {
        DEBUGF("logging:");
        LOGLET_DINDENT_SCOPE();
        logging::dump(config->logging);
    }

#ifdef DATA_TRACING
    {
        DEBUGF("data tracing:");
        LOGLET_DINDENT_SCOPE();
        data_tracing::dump(config->data_tracing);
    }
#endif
}

bool parse(int argc, char** argv, Config* config) {
    args::ArgumentParser parser("S3LP Client (" CLIENT_VERSION ")");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    args::GlobalOptions location_server_globals{parser, ls::gGroup};
    args::GlobalOptions identity_globals{parser, identity::gGroup};
    args::GlobalOptions assistance_data_globals{parser, ad::gGroup};
    args::GlobalOptions location_information_globals{parser, li::gGroup};
    args::GlobalOptions input_globals{parser, input::gGroup};
    args::GlobalOptions output_globals{parser, output::gGroup};
    args::GlobalOptions gnss_globals{parser, gnss::gGroup};
#ifdef INCLUDE_GENERATOR_RTCM
    args::GlobalOptions lpp2rtcm_globals{parser, lpp2rtcm::gGroup};
#endif
#ifdef INCLUDE_GENERATOR_SPARTN
    args::GlobalOptions lpp2spartn_globals{parser, lpp2spartn::gGroup};
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
    args::GlobalOptions tokoro_globals{parser, tokoro::gGroup};
#endif
    args::GlobalOptions logging_globals{parser, logging::gGroup};
#ifdef DATA_TRACING
    args::GlobalOptions data_tracing_globals{parser, data_tracing::gGroup};
#endif

    ls::setup();
    identity::setup();
    ad::setup();
    li::setup();
    input::setup();
    output::setup();
    gnss::setup();
#ifdef INCLUDE_GENERATOR_RTCM
    lpp2rtcm::setup();
#endif
#ifdef INCLUDE_GENERATOR_SPARTN
    lpp2spartn::setup();
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
    tokoro::setup();
#endif
    logging::setup();
#ifdef DATA_TRACING
    data_tracing::setup();
#endif

    // TODO(ewasjon): Use logger library here
    try {
        parser.ParseCLI(argc, argv);
        if (version) {
            std::cout << "S3LP Client (" << CLIENT_VERSION << ")" << std::endl;
            return true;
        }

        ls::parse(config);
        identity::parse(config);
        ad::parse(config);
        li::parse(config);
        input::parse(config);
        output::parse(config);
        gnss::parse(config);
#ifdef INCLUDE_GENERATOR_RTCM
        lpp2rtcm::parse(config);
#endif
#ifdef INCLUDE_GENERATOR_SPARTN
        lpp2spartn::parse(config);
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
        tokoro::parse(config);
#endif
        logging::parse(config);
#ifdef DATA_TRACING
        data_tracing::parse(config);
#endif

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

        dump(config);

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
