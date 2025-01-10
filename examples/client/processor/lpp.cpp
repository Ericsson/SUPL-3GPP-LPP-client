#include "lpp.hpp"

#include <sstream>

#include <loglet/loglet.hpp>
#include <lpp/session.hpp>

#ifdef INCLUDE_GENERATOR_RTCM
#include <generator/rtcm/generator.hpp>
#endif

#define LOGLET_CURRENT_MODULE "p/lpp"

void LppXerOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto xer_message = lpp::Session::encode_lpp_message_xer(message);
    auto data        = reinterpret_cast<uint8_t const*>(xer_message.c_str());
    auto size        = xer_message.size();

    for (auto const& output : mOutput.outputs) {
        if (!output.lpp_xer_support()) continue;
        if (output.print) {
            XINFOF(OUTPUT_PRINT_MODULE, "lpp-xer: %zd bytes", size);
        }
        output.interface->write(data, size);
    }
}

void LppUperOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto buffer = lpp::Session::encode_lpp_message(message);
    if (buffer.empty()) return;

    auto data = reinterpret_cast<uint8_t const*>(buffer.data());
    auto size = buffer.size();

    for (auto const& output : mOutput.outputs) {
        if (!output.lpp_uper_support()) continue;
        if (output.print) {
            XINFOF(OUTPUT_PRINT_MODULE, "lpp-uper: %zd bytes", size);
        }
        output.interface->write(data, size);
    }
}
