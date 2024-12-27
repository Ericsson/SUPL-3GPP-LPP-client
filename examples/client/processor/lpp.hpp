#pragma once
#include <memory>

#include <lpp/message.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"

namespace streamline {
template <>
struct Clone<lpp::Message> {
    lpp::Message operator()(lpp::Message const&) { __builtin_unreachable(); }
};
}  // namespace streamline

class LppXerOutput : public streamline::Inspector<lpp::Message> {
public:
    LppXerOutput(OutputConfig const& config) : mConfig(config) {}

    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;

private:
    OutputConfig const& mConfig;
};

class LppUperOutput : public streamline::Inspector<lpp::Message> {
public:
    LppUperOutput(OutputConfig const& config) : mConfig(config) {}

    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;

private:
    OutputConfig const& mConfig;
};

#ifdef INCLUDE_GENERATOR_RTCM
class LppRtcmFramedOutput : public streamline::Inspector<lpp::Message> {
public:
    LppRtcmFramedOutput(OutputConfig const& config, int lrf_rtcm_id)
        : mConfig(config), mRtcmId(lrf_rtcm_id) {}

    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;

private:
    OutputConfig const& mConfig;
    int                 mRtcmId;
};
#endif
