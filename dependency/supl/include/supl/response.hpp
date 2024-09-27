#pragma once
#include <core/core.hpp>

namespace supl {

struct RESPONSE {
    enum class PosMethod {
        agpsSETassisted     = 0,
        agpsSETbased        = 1,
        agpsSETassistedpref = 2,
        agpsSETbasedpref    = 3,
        autonomousGPS       = 4,
        aFLT                = 5,
        eCID                = 6,
        eOTD                = 7,
        oTDOA               = 8,
        noPosition          = 9,
        /* extension*/
        ver2_historicalDataRetrieval = 10,
        ver2_agnssSETassisted        = 11,
        ver2_agnssSETbased           = 12,
        ver2_agnssSETassistedpref    = 13,
        ver2_agnssSETbasedpref       = 14,
        ver2_autonomousGNSS          = 15,
        ver2_sessioninfoquery        = 16
    };

    PosMethod posMethod;
};

}  // namespace supl
