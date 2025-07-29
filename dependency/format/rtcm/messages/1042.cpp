#include "1042.hpp"

#include <bitset>
#include <loglet/loglet.hpp>
#include <helper.hpp>
#include <datafields.hpp>

LOGLET_MODULE3(format, rtcm, rtcm1042);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, rtcm, rtcm1042)

namespace format {
namespace rtcm {

Rtcm1042Message::Rtcm1042Message(std::vector<uint8_t> mData) NOEXCEPT
    : Message{mData} {}

void Rtcm1042Message::print() const NOEXCEPT {
}

std::unique_ptr<Message> Rtcm1042Message::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new Rtcm1042Message(*this));
}

std::unique_ptr<Message> Rtcm1042Message::parse(std::vector<uint8_t> mData) {
    if (mData.size()*8 < 8+16+512+24) {
        ERRORF("RTCM 1042 message created without enough data (requires %d bits, received %d bits)", 8+16+512+24, mData.size()*8);
        return std::make_unique<ErrorMessage>();
    }

    auto m = new Rtcm1042Message(mData);
    std::bitset<8+16+512+24> bits { 0UL };
    for (auto b : mData) {
        const std::bitset<8+16+512+24> bs {b};
        bits <<= 8;
        bits  |= bs;
    }

    std::size_t i = 8+16; 
    getdatafield(bits,i,  m->mType);
    if (m->mType != 1042) {
        ERRORF("RTCM 1042 message missmatched message number. should be '1042', was '%4d'", m->mType);
        ERRORF("bits: %s", bits.to_string().c_str());
        return std::make_unique<ErrorMessage>();
    }

    return std::unique_ptr<Rtcm1042Message>(m);
}

}  // namespace rtcm
}  // namespace format
