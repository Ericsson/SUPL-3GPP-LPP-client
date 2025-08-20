#include "config.hpp"

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

namespace idokeido {

static args::Group gGroup{"Idokeido:"};

static args::Flag gEnable{
    gGroup,
    "enable",
    "Enable Idokeido engine",
    {"idokeido"},
};

static void setup() {}

static void parse(Config* config) {
    auto& cfg   = config->idokeido;
    cfg.enabled = false;

    if (gEnable) cfg.enabled = true;
}

static void dump(IdokeidoConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;
}

}  // namespace idokeido
