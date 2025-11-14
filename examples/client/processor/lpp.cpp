#include "lpp.hpp"

#include <sstream>

#include <loglet/loglet.hpp>
#include <lpp/session.hpp>

#ifdef INCLUDE_GENERATOR_RTCM
#include <generator/rtcm/generator.hpp>
#endif

LOGLET_MODULE2(p, lpp);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, lpp)

void LppXerOutput::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto xer_message = lpp::Session::encode_lpp_message_xer(message);
    auto data        = reinterpret_cast<uint8_t const*>(xer_message.c_str());
    auto size        = xer_message.size();

    for (auto const& output : mOutput.outputs) {
        if (!output.lpp_xer_support()) continue;
        if (!output.accept_tag(tag)) {
            XVERBOSEF(OUTPUT_PRINT_MODULE, "lpp-xer: tag %llX not accepted", tag);
            continue;
        }
        XDEBUGF(OUTPUT_PRINT_MODULE, "lpp-xer: (%zd bytes) tag=%llX", size, tag);
        ASSERT(output.stage, "stage is null");
        output.stage->write(OUTPUT_FORMAT_LPP_XER, data, size);
    }
}

void LppUperOutput::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto buffer = lpp::Session::encode_lpp_message(message);
    if (buffer.empty()) return;

    auto data = reinterpret_cast<uint8_t const*>(buffer.data());
    auto size = buffer.size();

    for (auto const& output : mOutput.outputs) {
        if (!output.lpp_uper_support()) continue;
        if (!output.accept_tag(tag)) {
            XVERBOSEF(OUTPUT_PRINT_MODULE, "lpp-uper: tag %llX not accepted", tag);
            continue;
        }
        XDEBUGF(OUTPUT_PRINT_MODULE, "lpp-uper: (%zd bytes) tag=%llX", size, tag);
        ASSERT(output.stage, "stage is null");
        output.stage->write(OUTPUT_FORMAT_LPP_UPER, data, size);
    }
}
