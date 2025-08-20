#pragma once
#include <core/core.hpp>

namespace streamline {
class System;

template <typename T>
class Consumer {
public:
    using DataType = T;

    Consumer()                                                          = default;
    virtual ~Consumer()                                                 = default;
    virtual void        consume(System&, DataType&& data, uint64_t tag) = 0;
    virtual bool        accept(System&, uint64_t) { return true; }
    virtual char const* name() const NOEXCEPT = 0;
};
}  // namespace streamline
