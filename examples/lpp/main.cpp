#include <args.hpp>
#include <lpp/lpp.h>
#include "agnss_example.h"
#include "options.hpp"
#include "osr_example.h"
#include "ssr_example.h"

int main(int argc, char** argv) {
    // Initialize OpenSSL
    network_initialize();

    OptionParser parser{};
    parser.add_command(new osr_example::OsrCommand());
    parser.add_command(new ssr_example::SsrCommand());
    parser.add_command(new agnss_example::AgnssCommand());

    return parser.parse_and_execute(argc, argv);
}
