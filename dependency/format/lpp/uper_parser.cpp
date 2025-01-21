#include "uper_parser.hpp"

#include <cstdio>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wunused-function"
#include "A-GNSS-ProvideAssistanceData.h"
#include "Initiator.h"
#include "LPP-Message.h"
#include "LPP-TransactionID.h"
#pragma GCC diagnostic pop

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "f/lpp"

namespace format {
namespace lpp {

char const* UperParser::name() const NOEXCEPT {
    return "LPP-UPER";
}

LPP_Message* UperParser::try_parse() NOEXCEPT {
    // NOTE: Increase default max stack size to handle large messages.
    // TODO(ewasjon): Is this correct?
    asn_codec_ctx_t stack_ctx{};
    stack_ctx.max_stack_size = 1024 * 1024 * 4;

    if (buffer_length() < 6) {
        // not enough data for header
        VERBOSEF("not enough data for any message");
        return nullptr;
    }

    std::vector<uint8_t> buffer;
    buffer.resize(buffer_length());
    copy_to_buffer(buffer.data(), buffer.size());

    LPP_Message* message{};
    auto         result =
        uper_decode_complete(&stack_ctx, &asn_DEF_LPP_Message, reinterpret_cast<void**>(&message),
                             buffer.data(), buffer.size());
    if (result.code == RC_FAIL) {
        VERBOSEF("failed to decode uper: %zd bytes consumed (buffer %u)", result.consumed,
                 buffer_length());
        skip(result.consumed);
        ASN_STRUCT_FREE(asn_DEF_LPP_Message, message);
        return nullptr;
    } else if (result.code == RC_WMORE) {
        VERBOSEF("failed to decode uper: need more data, got %zd, consumed %zd (buffer %u)",
                 buffer.size(), result.consumed, buffer_length());
        ASN_STRUCT_FREE(asn_DEF_LPP_Message, message);
        return nullptr;
    } else {
        skip(result.consumed);
        VERBOSEF("decoded uper: %zd consumed (buffer %u)", result.consumed, buffer_length());
        return message;
    }
}

A_GNSS_ProvideAssistanceData* UperParser::try_parse_provide_assistance_data() NOEXCEPT {
    // NOTE: Increase default max stack size to handle large messages.
    // TODO(ewasjon): Is this correct?
    asn_codec_ctx_t stack_ctx{};
    stack_ctx.max_stack_size = 1024 * 1024 * 4;

    if (buffer_length() < 6) {
        // not enough data for header
        VERBOSEF("not enough data for any message");
        return nullptr;
    }

    std::vector<uint8_t> buffer;
    buffer.resize(buffer_length());
    copy_to_buffer(buffer.data(), buffer.size());

    A_GNSS_ProvideAssistanceData* message{};
    auto                          result =
        uper_decode_complete(&stack_ctx, &asn_DEF_A_GNSS_ProvideAssistanceData,
                             reinterpret_cast<void**>(&message), buffer.data(), buffer.size());
    if (result.code == RC_FAIL) {
        VERBOSEF("failed to decode uper: %zd bytes consumed (buffer %u)", result.consumed,
                 buffer_length());
        skip(result.consumed);
        ASN_STRUCT_FREE(asn_DEF_A_GNSS_ProvideAssistanceData, message);
        return nullptr;
    } else if (result.code == RC_WMORE) {
        VERBOSEF("failed to decode uper: need more data, got %zd, consumed %zd (buffer %u)",
                 buffer.size(), result.consumed, buffer_length());
        ASN_STRUCT_FREE(asn_DEF_A_GNSS_ProvideAssistanceData, message);
        return nullptr;
    } else {
        skip(result.consumed);
        VERBOSEF("decoded uper: %zd consumed (buffer %u)", result.consumed, buffer_length());
        return message;
    }
}

}  // namespace lpp
}  // namespace format
