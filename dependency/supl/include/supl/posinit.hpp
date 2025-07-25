#pragma once
#include <supl/common.hpp>

namespace supl {

struct POSINIT {
    SETCapabilities sETCapabilities;
    LocationID      locationID;
    std::vector<Payload> payloads;
};

}  // namespace supl
