#if defined(INCLUDE_GENERATOR_RTCM)
#include "lpp2frame_rtcm.hpp"

#include <generator/rtcm/generator.hpp>
#include <loglet/loglet.hpp>
#include <lpp/session.hpp>

LOGLET_MODULE2(p, l2f);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, l2f)

void Lpp2FrameRtcm::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto buffer = lpp::Session::encode_lpp_message(message);
    if (buffer.empty()) return;

    auto messages = generator::rtcm::Generator::generate_framing(mConfig.rtcm_message_id,
                                                                 buffer.data(), buffer.size());
    if (messages.empty()) {
        DEBUGF("no RTCM messages framed");
        return;
    }

    INFOF("framed %d RTCM messages", messages.size());
    DEBUG_INDENT_SCOPE();
    for (auto& submessage : messages) {
        auto sub_buffer = submessage.data().data();
        auto sub_size   = submessage.data().size();
        DEBUGF("message: %4u: %zu bytes", submessage.id(), sub_size);

        // TODO(ewasjon): These message should be passed back into the system
        for (auto const& output : mOutput.outputs) {
            auto output_lfr    = output.lfr_support();
            auto output_rtcm   = mConfig.output_in_rtcm && output.rtcm_support();
            auto should_output = output_rtcm || output_lfr;
            if (!should_output) continue;

            if (!output.accept_tag(tag)) {
                XDEBUGF(OUTPUT_PRINT_MODULE, "tag %llX not accepted", tag);
                continue;
            }

            if (output.print) {
                if (output_lfr) {
                    XINFOF(OUTPUT_PRINT_MODULE, "lfr : %04d (%zd bytes)", submessage.id(),
                           sub_size);
                }
                if (output_rtcm) {
                    XINFOF(OUTPUT_PRINT_MODULE, "rtcm: %04d (%zd bytes)", submessage.id(),
                           sub_size);
                }
            }

            ASSERT(output.stage, "stage is null");
            output.stage->write(OUTPUT_FORMAT_RTCM, sub_buffer, sub_size);
        }
    }
}

#endif
