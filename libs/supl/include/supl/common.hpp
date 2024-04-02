#pragma once

#include <supl/cell.hpp>
#include <supl/types.hpp>

#include <string>

namespace supl {

enum class PrefMethod {
    agpsSETassistedPreferred = 0,
    agpsSETBasedPreferred    = 1,
    noPreference             = 2,
};

struct PosProtocol {
    bool enabled;
    long majorVersionField;
    long technicalVersionField;
    long editorialVersionField;
};

struct SETCapabilities {
    struct {
        bool agpsSETassisted;
        bool agpsSETBased;
        bool autonomousGPS;
        bool aFLT;
        bool eCID;
        bool eOTD;
        bool oTDOA;
    } posTechnology;
    PrefMethod prefMethod;
    struct {
        PosProtocol lpp;
        PosProtocol rrlp;
        PosProtocol rrc;
    } posProtocol;
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
