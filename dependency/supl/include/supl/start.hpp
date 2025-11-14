#pragma once
#include <supl/common.hpp>

namespace supl {

struct START {
    SETCapabilities set_capabilities;
    ApplicationID   application_id;
    LocationID      location_id;
};

}  // namespace supl
