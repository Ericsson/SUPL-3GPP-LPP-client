#pragma once
#include <memory>

#include <format/rtcm/message.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"

namespace format {
namespace rtcm {
class Rtcm1019;
}
}  // namespace format

using RtcmMessage = std::unique_ptr<format::rtcm::Message>;

namespace streamline {
template <>
struct Clone<RtcmMessage> {
    RtcmMessage operator()(RtcmMessage const& value) { return value->clone(); }
};
}  // namespace streamline

class RtcmPrint : public streamline::Inspector<RtcmMessage> {
public:
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;
};

class RtcmOutput : public streamline::Inspector<RtcmMessage> {
public:
    RtcmOutput(OutputConfig const& output) : mOutput(output) {}

    const char* name() const NOEXCEPT override { return "RtcmOutput"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;

private:
    OutputConfig const& mOutput;
};
