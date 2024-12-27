#include "lpp.hpp"

#include <sstream>

#include <loglet/loglet.hpp>
#include <lpp/session.hpp>

#ifdef INCLUDE_GENERATOR_RTCM
#include <generator/rtcm/generator.hpp>
#endif

void LppXerOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    auto xer_message = lpp::Session::encode_lpp_message_xer(message);
    auto data        = reinterpret_cast<uint8_t const*>(xer_message.c_str());
    auto size        = xer_message.size();

    for (auto const& output : mConfig.outputs) {
        if ((output.format & OUTPUT_FORMAT_LPP_XER) == OUTPUT_FORMAT_LPP_XER) {
            output.interface->write(data, size);
        }
    }
}

void LppUperOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    auto buffer = lpp::Session::encode_lpp_message(message);
    if (buffer.empty()) return;

    auto data = reinterpret_cast<uint8_t const*>(buffer.data());
    auto size = buffer.size();

    for (auto const& output : mConfig.outputs) {
        if ((output.format & OUTPUT_FORMAT_LPP_UPER) == OUTPUT_FORMAT_LPP_UPER) {
            output.interface->write(data, size);
        }
    }
}

#ifdef INCLUDE_GENERATOR_RTCM
void LppRtcmFramedOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    auto buffer = lpp::Session::encode_lpp_message(message);
    if (buffer.empty()) return;

    auto submessages =
        generator::rtcm::Generator::generate_framing(mRtcmId, buffer.data(), buffer.size());
    for (auto& submessage : submessages) {
        // TODO(ewasjon): These message should be passed back into the system
        auto buffer = submessage.data().data();
        auto size   = submessage.data().size();
        for (auto const& output : mConfig.outputs) {
            if ((output.format & OUTPUT_FORMAT_LPP_RTCM_FRAME) != 0) {
                output.interface->write(buffer, size);
            }
        }
    }
}
#endif
