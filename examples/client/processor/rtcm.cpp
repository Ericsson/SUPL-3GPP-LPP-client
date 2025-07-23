#include "rtcm.hpp"

#include <format/rtcm/1019.hpp>
#include <loglet/loglet.hpp>
#include <lpp/location_information.hpp>

#include <cmath>
#include "config.hpp"

LOGLET_MODULE2(p, rtcm);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, rtcm)

void RtcmPrint::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    VSCOPE_FUNCTION();
    message->print();
}

void RtcmOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto& data = message->data();
    auto size = data.size();
    for (auto& output : mOutput.outputs) {
        if (!output.rtcm_support()) continue;
        if (output.print) {
            XINFOF(OUTPUT_PRINT_MODULE, "rtcm: %zd bytes", size);
        }

        output.interface->write(data.data(), size);
    }
}
