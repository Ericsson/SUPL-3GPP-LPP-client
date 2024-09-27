#pragma once
#include <lpp/types.hpp>

#include <functional>

namespace lpp {

enum class Initiator {
    TargetDevice,
    LocationServer,
};

}  // namespace lpp

template <>
struct std::hash<lpp::Initiator> {
    std::size_t operator()(const lpp::Initiator& k) const {
        return hash<long>()(k == lpp::Initiator::TargetDevice ? 0 : 1);
    }
};
