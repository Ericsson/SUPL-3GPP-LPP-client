#include "lpp.hpp"

#include <sstream>

#include <lpp/internal_lpp.h>
#include <lpp/lpp.h>

#ifdef INCLUDE_GENERATOR_RTCM
#include <generator/rtcm/generator.hpp>
#endif

namespace internal {
void LppMessageDeleter::operator()(LPP_Message* message) {
    ASN_STRUCT_FREE(asn_DEF_LPP_Message, message);
}
}  // namespace internal

void LppXerOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    std::stringstream buffer;
    xer_encode(
        &asn_DEF_LPP_Message, message.get(), XER_F_BASIC,
        [](void const* text_buffer, size_t text_size, void* app_key) -> int {
            auto string_stream = static_cast<std::ostream*>(app_key);
            string_stream->write(static_cast<char const*>(text_buffer),
                                 static_cast<std::streamsize>(text_size));
            return 0;
        },
        &buffer);
    auto xer_message = buffer.str();
    auto data        = reinterpret_cast<uint8_t const*>(xer_message.c_str());
    auto size        = xer_message.size();

    for (auto const& output : mOptions.outputs) {
        if ((output.format & OUTPUT_FORMAT_LPP_XER) == OUTPUT_FORMAT_LPP_XER) {
            output.interface->write(data, size);
        }
    }
}

void LppUperOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    auto octet = lpp_encode(message.get());
    if (octet) {
        auto data = reinterpret_cast<uint8_t const*>(octet->buf);
        auto size = octet->size;

        for (auto const& output : mOptions.outputs) {
            if ((output.format & OUTPUT_FORMAT_LPP_UPER) == OUTPUT_FORMAT_LPP_UPER) {
                output.interface->write(data, size);
            }
        }

        ASN_STRUCT_FREE(asn_DEF_OCTET_STRING, octet);
    }
}

#ifdef INCLUDE_GENERATOR_RTCM
void LppRtcmFramedOutput::inspect(streamline::System&,
                                  DataType const& message) NOEXCEPT {
    auto octet = lpp_encode(message.get());
    if (!octet) return;

    auto submessages =
        generator::rtcm::Generator::generate_framing(mRtcmId, octet->buf, octet->size);
    for (auto& submessage : submessages) {
        // TODO(ewasjon): These message should be passed back into the system
        auto buffer = submessage.data().data();
        auto size   = submessage.data().size();
        for (auto const& output : mOptions.outputs) {
            if ((output.format & OUTPUT_FORMAT_LPP_RTCM_FRAME) != 0) {
                output.interface->write(buffer, size);
            }
        }
    }

    ASN_STRUCT_FREE(asn_DEF_OCTET_STRING, octet);
}
#endif
