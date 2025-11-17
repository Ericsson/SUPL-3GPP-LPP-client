#include "messages/rxm_sfrbx.hpp"
#include "decoder.hpp"
#include "encoder.hpp"
#include "parser.hpp"

#include <cstdio>

#include <loglet/loglet.hpp>

LOGLET_MODULE3(ubx, msg, rxm_sfrbx);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(ubx, msg, rxm_sfrbx)

namespace format {
namespace ubx {

RxmSfrbx::RxmSfrbx(raw::RxmSfrbx payload, std::vector<uint32_t> words,
                   std::vector<uint8_t> data) NOEXCEPT
    : Message(CLASS_ID, MESSAGE_ID, std::move(data)),
      mPayload(std::move(payload)),
      mWords(std::move(words)) {}

static char const* message_type_str(raw::RxmSfrbx const& payload) NOEXCEPT {
    switch (payload.gnss_id) {
    case 0:  // GPS
        switch (payload.sig_id) {
        case 0: return "GPS L1 C/A";
        case 3: return "GPS L2 CL";
        case 4: return "GPS L2 CM";
        case 5: return "GPS L5 I";
        case 6: return "GPS L5 Q";
        default: return "GPS ?";
        }
    case 1:  // SBAS
        switch (payload.sig_id) {
        case 0: return "SBAS L1 C/A";
        default: return "SBAS ?";
        }
    case 2:  // Galileo
        switch (payload.sig_id) {
        case 0: return "Galileo E1 C";
        case 1: return "Galileo E1 B";
        case 3: return "Galileo E5a I";
        case 4: return "Galileo E5a Q";
        case 5: return "Galileo E5b I";
        case 6: return "Galileo E5b Q";
        default: return "Galileo ?";
        }
    case 3:  // BeiDou
        switch (payload.sig_id) {
        case 0: return "BeiDou B1I D1";
        case 1: return "BeiDou B1I D2";
        case 2: return "BeiDou B2I D1";
        case 3: return "BeiDou B2I D2";
        case 5: return "BeiDou B1 Cp (pilot)";
        case 6: return "BeiDou B1 Cd (data)";
        case 7: return "BeiDou B2 ap (pilot)";
        case 8: return "BeiDou B2a (data)";
        default: return "BeiDou ?";
        }
    case 5:  // QZSS
        switch (payload.sig_id) {
        case 0: return "QZSS L1 C/A";
        case 1: return "QZSS L1 S";
        case 4: return "QZSS L2 CM";
        case 5: return "QZSS L2 CL";
        case 8: return "QZSS L5 I";
        case 9: return "QZSS L5 Q";
        default: return "QZSS ?";
        }
    case 6:  // GLONASS
        switch (payload.sig_id) {
        case 0: return "GLONASS L1 OF";
        case 2: return "GLONASS L2 OF";
        default: return "GLONASS ?";
        }
    case 7:  // NavIC
        switch (payload.sig_id) {
        case 0: return "NavIC L5 A";
        default: return "NavIC ?";
        }
    default: return "?";
    }
}

void RxmSfrbx::print() const NOEXCEPT {
    printf("[%02X %02X] UBX-RXM-SRFBX: %u:%03u/%u \"%s\" %zu words\n", message_class(),
           message_id(), mPayload.gnss_id, mPayload.sv_id, mPayload.sig_id,
           message_type_str(mPayload), mWords.size());
}

std::unique_ptr<Message> RxmSfrbx::clone() const NOEXCEPT {
    return std::unique_ptr<Message>{new RxmSfrbx{*this}};
}

std::unique_ptr<Message> RxmSfrbx::parse(Decoder& decoder, std::vector<uint8_t> data) NOEXCEPT {
    if (decoder.remaining() < 8) {
        VERBOSEF("parse failed: insufficient data (need 8, have %u)", decoder.remaining());
        return nullptr;
    }

    auto gnss_id   = decoder.u1();
    auto sv_id     = decoder.u1();
    auto sig_id    = decoder.u1();
    auto freq_id   = decoder.u1();
    auto num_words = decoder.u1();
    auto chn       = decoder.u1();
    auto version   = decoder.u1();
    auto reserved0 = decoder.u1();
    if (decoder.error()) {
        VERBOSEF("parse failed: decoder error reading header");
        return nullptr;
    }

    raw::RxmSfrbx payload;
    payload.gnss_id   = gnss_id;
    payload.sv_id     = sv_id;
    payload.sig_id    = sig_id;
    payload.freq_id   = freq_id;
    payload.num_words = num_words;
    payload.chn       = chn;
    payload.version   = version;
    payload.reserved0 = reserved0;
    if (version != 0x02) {
        VERBOSEF("parse failed: unsupported version %u", version);
        return nullptr;
    }

    std::vector<uint32_t> words;
    for (size_t i = 0; i < num_words; ++i) {
        words.push_back(decoder.u4());
    }

    if (decoder.error()) {
        VERBOSEF("parse failed: decoder error reading words");
        return nullptr;
    } else {
        return std::unique_ptr<Message>{
            new RxmSfrbx(std::move(payload), std::move(words), std::move(data))};
    }
}

}  // namespace ubx
}  // namespace format
