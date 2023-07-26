#pragma once
#include "target.h"

#include <string>

class StdoutTarget final : public TransmitterTarget {
public:
    StdoutTarget();
    ~StdoutTarget();

    void transmit(const void* data, const size_t size) override;
};
