#include <loglet/loglet.hpp>
#include "../config.hpp"

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

namespace identity {

static args::Group gGroup{"Identity:"};
static args::Flag  gWaitForIdentity{
    gGroup,
    "identity-wait",
    "Wait for the identity to be provided via the control interface",
     {"wait-for-identity"},
    args::Options::Single,
};
static args::ValueFlag<unsigned long long> gMsisdn{
    gGroup, "msisdn", "Mobile Subscriber ISDN", {"msisdn"}, args::Options::Single,
};
static args::ValueFlag<unsigned long long> gImsi{
    gGroup, "imsi", "International Mobile Subscriber Identity", {"imsi"}, args::Options::Single,
};
static args::ValueFlag<std::string> gIpv4{
    gGroup, "ipv4", "IPv4 address", {"ipv4"}, args::Options::Single,
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions sGlobals{parser, gGroup};
}

void parse(Config* config) {
    if (gWaitForIdentity) config->identity.wait_for_identity = true;

    if (!config->location_server.enabled && !config->agnss.enabled) {
        VERBOSEF("identity is only required when connecting to the location server or A-GNSS\n");
        return;
    }

    if (!gWaitForIdentity && !gMsisdn && !gImsi && !gIpv4) {
        throw args::RequiredError(
            "identity is required, use `--identity-wait` or `--msisdn` or `--imsi` or `--ipv4`");
    }

    if (gMsisdn) {
        config->identity.msisdn = std::unique_ptr<uint64_t>{new uint64_t{gMsisdn.Get()}};
    }

    if (gImsi) {
        config->identity.imsi = std::unique_ptr<uint64_t>{new uint64_t{gImsi.Get()}};
    }

    if (gIpv4) {
        config->identity.ipv4 = std::unique_ptr<std::string>{new std::string{gIpv4.Get()}};
    }
}

void dump(IdentityConfig const& config) {
    DEBUGF("wait_for_identity: %s", config.wait_for_identity ? "true" : "false");

    if (config.msisdn) {
        DEBUGF("msisdn: %llu", *config.msisdn);
    } else {
        DEBUGF("msisdn: not set");
    }

    if (config.imsi) {
        DEBUGF("imsi: %llu", *config.imsi);
    } else {
        DEBUGF("imsi: not set");
    }

    if (config.ipv4) {
        DEBUGF("ipv4: \"%s\"", config.ipv4->c_str());
    } else {
        DEBUGF("ipv4: not set");
    }
}

}  // namespace identity
