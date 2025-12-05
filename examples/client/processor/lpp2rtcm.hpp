#pragma once

#if !defined(INCLUDE_GENERATOR_RTCM)
#error "INCLUDE_GENERATOR_RTCM must be defined"
#endif

#include <generator/rtcm/generator.hpp>
#include <scheduler/scheduler.hpp>

#include "config.hpp"
#include "lpp.hpp"

class Lpp2Rtcm : public streamline::Inspector<lpp::Message> {
public:
    Lpp2Rtcm(OutputConfig const& output, Lpp2RtcmConfig const& config,
             scheduler::Scheduler& scheduler);
    ~Lpp2Rtcm() override;

    NODISCARD char const* name() const NOEXCEPT override { return "Lpp2Rtcm"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) override;

private:
    std::unique_ptr<generator::rtcm::Generator> mGenerator;
    generator::rtcm::MessageFilter              mFilter;

    OutputConfig const&   mOutput;
    Lpp2RtcmConfig const& mConfig;
    scheduler::Scheduler& mScheduler;
    size_t                mConversionCount;
};
