#if defined(INCLUDE_GENERATOR_RTCM)
#include "lpp2rtcm.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(p, l2r);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, l2r)

Lpp2Rtcm::Lpp2Rtcm(OutputConfig const& output, Lpp2RtcmConfig const& config)
    : mOutput(output), mConfig(config) {
    VSCOPE_FUNCTION();
    mGenerator = std::unique_ptr<generator::rtcm::Generator>(new generator::rtcm::Generator{});
    mFilter    = generator::rtcm::MessageFilter{};
    mFilter.systems.gps     = config.generate_gps;
    mFilter.systems.glonass = config.generate_glonass;
    mFilter.systems.galileo = config.generate_galileo;
    mFilter.systems.beidou  = config.generate_beidou;

    if (config.msm_type == Lpp2RtcmConfig::MsmType::MSM4) mFilter.msm.force_msm4 = true;
    if (config.msm_type == Lpp2RtcmConfig::MsmType::MSM5) mFilter.msm.force_msm5 = true;
    if (config.msm_type == Lpp2RtcmConfig::MsmType::MSM6) mFilter.msm.force_msm6 = true;
    if (config.msm_type == Lpp2RtcmConfig::MsmType::MSM7) mFilter.msm.force_msm7 = true;
}

Lpp2Rtcm::~Lpp2Rtcm() {
    VSCOPE_FUNCTION();
}

void Lpp2Rtcm::inspect(streamline::System&, DataType const& message, uint64_t tag) {
    VSCOPE_FUNCTION();
    auto messages = mGenerator->generate(message.get(), mFilter);
    if (messages.empty()) {
        WARNF("no RTCM messages generated, check that you're using `--ad-type osr`");
        return;
    }

    INFOF("generated %d RTCM messages", messages.size());
    DEBUG_INDENT_SCOPE();
    for (auto& submessage : messages) {
        auto buffer = submessage.data().data();
        auto size   = submessage.data().size();
        DEBUGF("message: %4u: %zu bytes", submessage.id(), size);

        // TODO(ewasjon): These message should be passed back into the system
        for (auto const& output : mOutput.outputs) {
            if (!output.rtcm_support()) continue;
            if (!output.accept_tag(tag)) {
                XDEBUGF(OUTPUT_PRINT_MODULE, "tag %llX not accepted", tag);
                continue;
            }
            if (output.print) {
                XINFOF(OUTPUT_PRINT_MODULE, "rtcm: %04d (%zd bytes)", submessage.id(), size);
            }

            ASSERT(output.stage, "stage is null");
            output.stage->write(OUTPUT_FORMAT_RTCM, buffer, size);
        }
    }
}

#endif
