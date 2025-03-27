
namespace logging {

static args::Group gGroup{"Logging:"};
static args::Flag  gTrace{
    gGroup,
    "trace",
    "Set log level to trace",
     {"trace"},
};
static args::Flag gVerbose{
    gGroup,
    "verbose",
    "Set log level to verbose",
    {"verbose"},
};
static args::Flag gDebug{
    gGroup,
    "debug",
    "Set log level to debug",
    {"debug"},
};
static args::Flag gInfo{
    gGroup,
    "info",
    "Set log level to info",
    {"info"},
};
static args::Flag gNotice{
    gGroup,
    "notice",
    "Set log level to notice",
    {"notice"},
};
static args::Flag gWarning{
    gGroup,
    "warning",
    "Set log level to warning",
    {"warning"},
};
static args::Flag gError{
    gGroup,
    "error",
    "Set log level to error",
    {"error"},
};
static args::ValueFlagList<std::string> gModules{
    gGroup,
    "module",
    "<module>=<level>",
    {"lm"},
};
static args::Flag gNoColor{
    gGroup,
    "no-color",
    "Disable colored output",
    {"log-no-color"},
};
static args::Flag gFlush{
    gGroup,
    "flush",
    "Flush log after each line",
    {"log-flush"},
};
static args::Flag gTree{
    gGroup,
    "tree",
    "Show log tree",
    {"log-tree"},
};

static void setup() {}

static void parse(Config* config) {
    auto& logging     = config->logging;
    logging.log_level = loglet::Level::Info;
    logging.color     = gNoColor ? false : true;
    logging.flush     = gFlush;
    logging.tree      = gTree;

    if (gTrace) {
        logging.log_level = loglet::Level::Trace;
    } else if (gVerbose) {
        logging.log_level = loglet::Level::Verbose;
    } else if (gDebug) {
        logging.log_level = loglet::Level::Debug;
    } else if (gInfo) {
        logging.log_level = loglet::Level::Info;
    } else if (gNotice) {
        logging.log_level = loglet::Level::Notice;
    } else if (gWarning) {
        logging.log_level = loglet::Level::Warning;
    } else if (gError) {
        logging.log_level = loglet::Level::Error;
    }

    for (auto const& module : gModules) {
        auto parts = split(module, '=');
        if (parts.size() != 2) {
            throw args::ValidationError("invalid log module: `" + module + "`");
        }

        auto level = loglet::Level::Disabled;
        if (parts[1] == "trace") {
            level = loglet::Level::Trace;
        } else if (parts[1] == "verbose") {
            level = loglet::Level::Verbose;
        } else if (parts[1] == "debug") {
            level = loglet::Level::Debug;
        } else if (parts[1] == "info") {
            level = loglet::Level::Info;
        } else if (parts[1] == "notice") {
            level = loglet::Level::Notice;
        } else if (parts[1] == "warning") {
            level = loglet::Level::Warning;
        } else if (parts[1] == "error") {
            level = loglet::Level::Error;
        } else if (parts[1] == "disable") {
            level = loglet::Level::Disabled;
        } else {
            throw args::ValidationError("invalid log level: `" + parts[1] + "`");
        }

        logging.module_levels[parts[0]] = level;
    }
}

static char const* level_to_string(loglet::Level level) {
    switch (level) {
    case loglet::Level::Trace: return "trace";
    case loglet::Level::Verbose: return "verbose";
    case loglet::Level::Debug: return "debug";
    case loglet::Level::Info: return "info";
    case loglet::Level::Notice: return "notice";
    case loglet::Level::Warning: return "warning";
    case loglet::Level::Error: return "error";
    case loglet::Level::Disabled: return "disabled";
    }
}

static void dump(LoggingConfig const& config) {
    DEBUGF("log level: %s", level_to_string(config.log_level));
    for (auto const& [module, level] : config.module_levels) {
        DEBUGF("module %s: %s", module.c_str(), level_to_string(level));
    }
}

}  // namespace logging
