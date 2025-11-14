#pragma once
#include <supl/common.hpp>

namespace supl {

struct POSINIT {
    SETCapabilities      set_capabilities;
    LocationID           location_id;
    std::vector<Payload> payloads;
};

}  // namespace supl
