#pragma once
#include <streamline/inspector.hpp>

#include "../config.hpp"
#include "../raw_message.hpp"

class RawOutput : public streamline::Inspector<RawMessage> {
public:
    RawOutput(OutputConfig const& output) : mOutput(output) {}

    NODISCARD char const* name() const NOEXCEPT override { return "RawOutput"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;

private:
    OutputConfig const& mOutput;
};
