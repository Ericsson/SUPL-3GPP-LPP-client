#pragma once
#include <supl/common.hpp>

namespace supl {

struct START {
    SETCapabilities sETCapabilities;
    ApplicationID   applicationID;
    LocationID      locationID;
};

}  // namespace supl
