#include "rtcm.hpp"

#include <format/rtcm/1019.hpp>
#include <loglet/loglet.hpp>
#include <lpp/location_information.hpp>

#include <cmath>
#include "config.hpp"

LOGLET_MODULE2(p, rtcm);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, rtcm)

void RtcmPrint::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();
    for (auto const& print : mConfig.prints) {
        if (!print.rtcm_support()) continue;
        if (!print.accept_tag(tag)) continue;
        message->print();
        return;
    }
}

void RtcmOutput::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto& data = message->data();
    auto  size = data.size();
    for (auto& output : mOutput.outputs) {
        if (!output.rtcm_support()) continue;
        if (!output.accept_tag(tag)) {
            XVERBOSEF(OUTPUT_PRINT_MODULE, "tag %llX not accepted", tag);
            continue;
        }
        XDEBUGF(OUTPUT_PRINT_MODULE, "rtcm: %04d (%zd bytes) tag=%llX", message->type(), size, tag);

        ASSERT(output.stage, "stage is null");
        output.stage->write(OUTPUT_FORMAT_RTCM, data.data(), size);
    }
}
