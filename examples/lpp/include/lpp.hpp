#pragma once
#include <memory>

#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "options.hpp"

struct LPP_Message;

namespace internal {
struct LppMessageDeleter {
    void operator()(LPP_Message* message);
};
}  // namespace internal

using LppMessage = std::unique_ptr<LPP_Message, internal::LppMessageDeleter>;

namespace streamline {
template <>
struct Clone<LppMessage> {
    LppMessage operator()(LppMessage const&) { __builtin_unreachable(); }
};
}  // namespace streamline

class LppXerOutput : public streamline::Inspector<LppMessage> {
public:
    LppXerOutput(OutputOptions const& options) : mOptions(options) {}

    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;

private:
    OutputOptions const& mOptions;
};

class LppUperOutput : public streamline::Inspector<LppMessage> {
public:
    LppUperOutput(OutputOptions const& options) : mOptions(options) {}

    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;

private:
    OutputOptions const& mOptions;
};

#ifdef INCLUDE_GENERATOR_RTCM
class LppRtcmFramedOutput : public streamline::Inspector<LppMessage> {
public:
    LppRtcmFramedOutput(OutputOptions const& options, int lrf_rtcm_id)
        : mOptions(options), mRtcmId(lrf_rtcm_id) {}

    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;

private:
    OutputOptions const& mOptions;
    int                  mRtcmId;
};
#endif
