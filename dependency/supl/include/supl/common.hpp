#pragma once
#include <supl/cell.hpp>

#include <string>

namespace supl {

enum class PrefMethod {
    AgpsSETAssistedPreferred = 0,
    AgpsSETBasedPreferred    = 1,
    NoPreference             = 2,
};

struct PosProtocol {
    bool enabled;
    long major_version_field;
    long technical_version_field;
    long editorial_version_field;
};

struct SETCapabilities {
    struct {
        bool agps_set_assisted;
        bool agps_set_based;
        bool autonomous_gps;
        bool aflt;
        bool ecid;
        bool eotd;
        bool otdoa;
    } pos_technology;
    PrefMethod pref_method;
    struct {
        PosProtocol lpp;
        PosProtocol rrlp;
        PosProtocol rrc;
    } pos_protocol;
};

struct LocationID {
    Cell cell;
};

struct ApplicationID {
    std::string name;
    std::string provider;
    std::string version;
};

}  // namespace supl
