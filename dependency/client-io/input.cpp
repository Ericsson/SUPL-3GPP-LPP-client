#include <client-io/parse.hpp>
#include <client-io/registry.hpp>
#include <client-io/types.hpp>

#include <core/string.hpp>
#include <loglet/loglet.hpp>

#include <client-io/program_io.hpp>

LOGLET_MODULE(client_io_input);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(client_io_input)

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

namespace input {

static args::Group*                      gGroup                         = nullptr;
static args::ValueFlagList<std::string>* gArgs                          = nullptr;
static args::Flag*                       gDisablePipeBufferOptimization = nullptr;
static args::Flag*                       gShutdownOnComplete            = nullptr;
static args::Flag*                       gSyncMode                      = nullptr;
static args::ValueFlag<int>*             gShutdownDelay                 = nullptr;

void setup(args::ArgumentParser& parser) {
    // Build help text from registry
    std::string help = "Add an input interface.\n"
                       "Usage: --input <type>:<arguments>\n\n"
                       "Types:\n";

    for (auto const& t : io_registry::input_types()) {
        help += "  " + t.name + ":\n";
        if (!t.help.empty()) help += t.help;
    }

    if (!io_registry::input_formats().empty()) {
        help += "\nOptions:\n  format=<fmt>[+<fmt>...]  ";
        for (size_t i = 0; i < io_registry::input_formats().size(); i++) {
            if (i) help += ", ";
            help += io_registry::input_formats()[i].name;
        }
        help += "\n";
    }
    help += "  tags=<tag>[+<tag>...]\n"
            "  unique=<bool> (default=false)\n"
            "  exclude_from_shutdown=<bool> (default=false)\n";

    gGroup = new args::Group{"Input:"};
    gArgs  = new args::ValueFlagList<std::string>{*gGroup, "input", help, {"input"}};
    gDisablePipeBufferOptimization = new args::Flag{*gGroup,
                                                    "disable-pipe-buffer-optimization",
                                                    "Disable pipe buffer size optimization",
                                                    {"input-disable-pipe-buffer-optimization"}};
    gShutdownOnComplete            = new args::Flag{*gGroup,
                                         "shutdown-on-complete",
                                         "Shutdown when all inputs complete",
                                                    {"input-shutdown-on-complete"}};
    gSyncMode                      = new args::Flag{
        *gGroup, "sync-mode", "Synchronous dispatch (faster post-processing)", {"input-sync-mode"}};
    gShutdownDelay = new args::ValueFlag<int>{*gGroup,
                                              "milliseconds",
                                              "Delay before shutdown (default: 1000ms)",
                                              {"input-shutdown-delay"},
                                              1000};

    static args::GlobalOptions sGlobals{parser, *gGroup};
}

void parse(InputsConfig& config) {
    if (!gArgs) return;

    config.disable_pipe_buffer_optimization = gDisablePipeBufferOptimization->Get();
    config.shutdown_on_complete             = gShutdownOnComplete->Get();
    config.sync_mode                        = gSyncMode->Get();
    config.shutdown_delay                   = std::chrono::milliseconds(gShutdownDelay->Get());

    for (auto const& arg : gArgs->Get()) {
        auto colon = arg.find(':');
        auto type  = colon == std::string::npos ? arg : arg.substr(0, colon);
        auto opts  = colon == std::string::npos ? io_registry::Options{} :
                                                  parse_options(arg.substr(colon + 1));

        InputEntry entry;
        entry.type    = type;
        entry.options = opts;

        if (opts.count("format") && !io_registry::input_formats().empty()) {
            entry.format = io_registry::parse_input_format_list(opts.at("format"));
        }
        if (opts.count("tags")) {
            entry.tags = parse_list(opts.at("tags"), '+');
        }
        entry.print                 = parse_bool(opts, "print", false);
        entry.nmea_lf_only          = parse_bool(opts, "nmea_lf_only", false);
        entry.discard_errors        = parse_bool(opts, "discard_errors", false);
        entry.discard_unknowns      = parse_bool(opts, "discard_unknowns", false);
        entry.exclude_from_shutdown = parse_bool(opts, "exclude_from_shutdown", false);

        config.inputs.push_back(std::move(entry));
    }
}

void dump(ProgramInput const& config) {
    for (auto const& input : config.inputs) {
        DEBUGF("input type=%s format=0x%llx", input.entry.type.c_str(),
               (unsigned long long)input.entry.format);
    }
}

void dump(InputsConfig const& config) {
    for (auto const& e : config.inputs) {
        DEBUGF("input: type=%s", e.type.c_str());
    }
}

}  // namespace input
