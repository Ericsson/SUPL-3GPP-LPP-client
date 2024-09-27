#pragma once
#include <core/core.hpp>

namespace streamline {
class System;

template <typename T>
class Inspector {
public:
    using DataType = T;

    Inspector()                                         = default;
    virtual ~Inspector()                                = default;
    virtual void inspect(System&, DataType const& data) = 0;
};
}  // namespace streamline
