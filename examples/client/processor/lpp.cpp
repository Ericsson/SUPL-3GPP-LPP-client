#include "lpp.hpp"

#include <sstream>

#include <loglet/loglet.hpp>
#include <lpp/session.hpp>

#ifdef INCLUDE_GENERATOR_RTCM
#include <generator/rtcm/generator.hpp>
#endif

LOGLET_MODULE2(p, lpp);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, lpp)

void LppXerOutput::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto xer_message = lpp::Session::encode_lpp_message_xer(message);
    auto data        = reinterpret_cast<uint8_t const*>(xer_message.c_str());
    auto size        = xer_message.size();

    for (auto const& output : mOutput.outputs) {
        if (!output.lpp_xer_support()) continue;
        if(!output.accept_tag(tag)) {
            XDEBUGF(OUTPUT_PRINT_MODULE, "lpp-xer: tag %llX not accepted", tag);
            continue;
        }
        if (output.print) {
            XINFOF(OUTPUT_PRINT_MODULE, "lpp-xer: %zd bytes", size);
        }
        output.interface->write(data, size);
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
        if(!output.accept_tag(tag)) {
            XDEBUGF(OUTPUT_PRINT_MODULE, "lpp-uper: tag %llX not accepted", tag);
            continue;
        }
        if (output.print) {
            XINFOF(OUTPUT_PRINT_MODULE, "lpp-uper: %zd bytes", size);
        }
        output.interface->write(data, size);
    }
}
