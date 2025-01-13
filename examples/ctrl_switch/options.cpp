#include "options.hpp"

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

static args::Group                       arguments{"Arguments:"};
static args::PositionalList<std::string> commands{arguments, "commands", "List of commands"};

Config parse_configuration(int argc, char** argv) {
    args::ArgumentParser parser(
        "Control Switch Example (" CLIENT_VERSION
        ") - This example demonstrates how to create a simple controller. This controller will "
        "send the input commands one by one (in a loop). Use with example-lpp with `--ctrl-tcp "
        "localhost --ctrl-tcp-port 13226`.");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    parser.Add(arguments);

    try {
        parser.ParseCLI(argc, argv);

        if (version) {
            std::cout << "Control Switch Example (" << CLIENT_VERSION << ")" << std::endl;
            exit(0);
        }

        Config config{};
        config.commands = commands.Get();
        return config;
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
