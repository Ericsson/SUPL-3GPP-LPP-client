#pragma once
#include <core/core.hpp>

namespace streamline {
class System;

template <typename T>
class Inspector {
public:
    using DataType = T;

    Inspector()                                                              = default;
    virtual ~Inspector()                                                     = default;
    virtual void        inspect(System&, DataType const& data, uint64_t tag) = 0;
    virtual bool        accept(System&, uint64_t) { return true; }
    virtual char const* name() const NOEXCEPT = 0;
};
}  // namespace streamline
