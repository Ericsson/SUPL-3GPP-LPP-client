#pragma once

#if !defined(INCLUDE_GENERATOR_RTCM)
#error "INCLUDE_GENERATOR_RTCM must be defined"
#endif

#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"
#include "lpp.hpp"

class Lpp2FrameRtcm : public streamline::Inspector<lpp::Message> {
public:
    Lpp2FrameRtcm(OutputConfig const& output, Lpp2FrameRtcmConfig const& config)
        : mOutput(output), mConfig(config) {}

    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;

private:
    OutputConfig const& mOutput;
    Lpp2FrameRtcmConfig mConfig;
};
