#pragma once
#include <core/core.hpp>

#include <functional>

namespace lpp {

enum class Initiator {
    TargetDevice,
    LocationServer,
};

}  // namespace lpp

template <>
struct std::hash<lpp::Initiator> {
    std::size_t operator()(lpp::Initiator const& k) const {
        return hash<long>()(k == lpp::Initiator::TargetDevice ? 0 : 1);
    }
};
