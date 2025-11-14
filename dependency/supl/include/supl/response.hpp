#pragma once
#include <core/core.hpp>

namespace supl {

struct RESPONSE {
    enum class PosMethod {
        AgpsSETAssisted     = 0,
        AgpsSETBased        = 1,
        AgpsSETAssistedPref = 2,
        AgpsSETBasedPref    = 3,
        AutonomousGPS       = 4,
        AFLT                = 5,
        ECID                = 6,
        EOTD                = 7,
        OTDOA               = 8,
        NoPosition          = 9,
        /* extension*/
        Ver2HistoricalDataRetrieval = 10,
        Ver2AgnssSETAssisted        = 11,
        Ver2AgnssSETBased           = 12,
        Ver2AgnssSETAssistedPref    = 13,
        Ver2AgnssSETBasedPref       = 14,
        Ver2AutonomousGNSS          = 15,
        Ver2SessionInfoQuery        = 16
    };

    PosMethod pos_method;
};

}  // namespace supl
