#pragma once
#include <core/core.hpp>

namespace streamline {
class System;

template <typename T>
class Consumer {
public:
    using DataType = T;

    Consumer()                            = default;
    virtual ~Consumer()                   = default;
    virtual void consume(System&, DataType&& data) = 0;
};
}  // namespace streamline
