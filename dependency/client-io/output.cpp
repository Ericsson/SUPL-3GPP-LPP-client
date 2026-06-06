#include <client-io/parse.hpp>
#include <client-io/registry.hpp>
#include <client-io/types.hpp>

#include <core/string.hpp>
#include <loglet/loglet.hpp>

#include <client-io/program_io.hpp>

LOGLET_MODULE(client_io_output);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(client_io_output)

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

namespace output {

static args::Group*                      gGroup = nullptr;
static args::ValueFlagList<std::string>* gArgs  = nullptr;

void setup(args::ArgumentParser& parser) {
    std::string help = "Add an output interface.\n"
                       "Usage: --output <type>:<arguments>\n\n"
                       "Types:\n";

    for (auto const& t : io_registry::output_types()) {
        help += "  " + t.name + ":\n";
        if (!t.help.empty()) help += t.help;
    }

    if (!io_registry::output_formats().empty()) {
        help += "\nOptions:\n  format=<fmt>[+<fmt>...]  ";
        for (size_t i = 0; i < io_registry::output_formats().size(); i++) {
            if (i) help += ", ";
            help += io_registry::output_formats()[i].name;
        }
        help += "\n";
    }
    help += "  itags=<tag>[+<tag>...]\n"
            "  otags=<tag>[+<tag>...]\n"
            "  unique=<bool> (default=false)\n";

    gGroup = new args::Group{"Output:"};
    gArgs  = new args::ValueFlagList<std::string>{*gGroup, "output", help, {"output"}};

    static args::GlobalOptions sGlobals{parser, *gGroup};
}

void parse(OutputsConfig& config) {
    if (!gArgs) return;

    for (auto const& arg : gArgs->Get()) {
        auto colon = arg.find(':');
        auto type  = colon == std::string::npos ? arg : arg.substr(0, colon);
        auto opts  = colon == std::string::npos ? io_registry::Options{} :
                                                  parse_options(arg.substr(colon + 1));

        OutputEntry entry;
        entry.type    = type;
        entry.options = opts;

        if (opts.count("format") && !io_registry::output_formats().empty()) {
            entry.format = io_registry::parse_output_format_list(opts.at("format"));
        }
        if (opts.count("itags")) entry.include_tags = parse_list(opts.at("itags"), '+');
        if (opts.count("otags")) entry.exclude_tags = parse_list(opts.at("otags"), '+');

        config.outputs.push_back(std::move(entry));
    }
}

void dump(ProgramOutput const& config) {
    for (auto const& o : config.outputs) {
        DEBUGF("output type=%s format=0x%llx", o.entry.type.c_str(),
               (unsigned long long)o.entry.format);
    }
}

void dump(OutputsConfig const& config) {
    for (auto const& e : config.outputs) {
        DEBUGF("output: type=%s", e.type.c_str());
    }
}

}  // namespace output
