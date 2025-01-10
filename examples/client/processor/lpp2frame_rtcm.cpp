#if defined(INCLUDE_GENERATOR_RTCM)
#include "lpp2frame_rtcm.hpp"

#include <generator/rtcm/generator.hpp>
#include <loglet/loglet.hpp>
#include <lpp/session.hpp>

#define LOGLET_CURRENT_MODULE "p/l2f"

void Lpp2FrameRtcm::inspect(streamline::System&, DataType const& message) NOEXCEPT {
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
    LOGLET_DINDENT_SCOPE();
    for (auto& submessage : messages) {
        auto buffer = submessage.data().data();
        auto size   = submessage.data().size();
        DEBUGF("message: %4u: %zu bytes", submessage.id(), size);

        // TODO(ewasjon): These message should be passed back into the system
        for (auto const& output : mOutput.outputs) {
            if (!output.rtcm_support()) continue;
            if (output.print) {
                XINFOF(OUTPUT_PRINT_MODULE, "rtcm: %04d (%zd bytes)", submessage.id(), size);
            }

            output.interface->write(buffer, size);
        }
    }
}

#endif
