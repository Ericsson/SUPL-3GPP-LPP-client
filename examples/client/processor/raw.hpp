#pragma once
#include <streamline/inspector.hpp>
#include "../program_io.hpp"

#include "../config.hpp"
#include "../raw_message.hpp"

class RawOutput : public streamline::Inspector<RawMessage> {
public:
    RawOutput(ProgramOutput const& output) : mOutput(output) {}

    NODISCARD char const* name() const NOEXCEPT override { return "RawOutput"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;

private:
    ProgramOutput const& mOutput;
};
