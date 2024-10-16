#include "options.hpp"

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

using namespace interface;

Options parse_configuration(int argc, char** argv) {
    args::ArgumentParser parser("Decode UPER Example (" CLIENT_VERSION ")");
    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    try {
        parser.ParseCLI(argc, argv);

        if (version) {
            std::cout << "Decode UPER Example (" CLIENT_VERSION << ")" << std::endl;
            exit(0);
        }

        Options options{};
        return options;
    } catch (args::ValidationError const& e) {
        std::cerr << e.what() << std::endl;
        parser.Help(std::cerr);
        exit(1);
    } catch (args::Help const&) {
        std::cout << parser;
        exit(0);
    } catch (args::ParseError const& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        exit(1);
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit(1);
    }
}
