#include "ubx_cfg.hpp"
#include "decoder.hpp"

namespace receiver {
namespace ublox {

CfgValue CfgValue::from_l(bool value) UBLOX_NOEXCEPT {
    return CfgValue(Type::L, value);
}

CfgValue CfgValue::from_u1(uint8_t value) UBLOX_NOEXCEPT {
    return CfgValue(Type::U1, value);
}

CfgValue CfgValue::from_u2(uint16_t value) UBLOX_NOEXCEPT {
    return CfgValue(Type::U2, value);
}

CfgValue CfgValue::from_u4(uint32_t value) UBLOX_NOEXCEPT {
    return CfgValue(Type::U4, value);
}

CfgValue CfgValue::from_u8(uint64_t value) UBLOX_NOEXCEPT {
    return CfgValue(Type::U8, value);
}

CfgValue::CfgValue(Type type, bool value) UBLOX_NOEXCEPT : mType(type) {
    mValue.mL = value;
}

CfgValue::CfgValue(Type type, uint8_t value) UBLOX_NOEXCEPT : mType(type) {
    mValue.mU1 = value;
}

CfgValue::CfgValue(Type type, uint16_t value) UBLOX_NOEXCEPT : mType(type) {
    mValue.mU2 = value;
}

CfgValue::CfgValue(Type type, uint32_t value) UBLOX_NOEXCEPT : mType(type) {
    mValue.mU4 = value;
}

CfgValue::CfgValue(Type type, uint64_t value) UBLOX_NOEXCEPT : mType(type) {
    mValue.mU8 = value;
}

//
//
//

bool CfgValue::l() const UBLOX_NOEXCEPT {
    return mValue.mL;
}

uint8_t CfgValue::u1() const UBLOX_NOEXCEPT {
    return mValue.mU1;
}

uint16_t CfgValue::u2() const UBLOX_NOEXCEPT {
    return mValue.mU2;
}

uint32_t CfgValue::u4() const UBLOX_NOEXCEPT {
    return mValue.mU4;
}

uint64_t CfgValue::u8() const UBLOX_NOEXCEPT {
    return mValue.mU8;
}

//
//
//

CfgValue CfgValue::parse_from_key(CfgKey key, Decoder& decoder) UBLOX_NOEXCEPT {
    auto size_bits = (key & 0x70000000U) >> 28;
    switch (size_bits) {
    case 1: return parse_from_type(Type::L, decoder);
    case 2: return parse_from_type(Type::U1, decoder);
    case 3: return parse_from_type(Type::U2, decoder);
    case 4: return parse_from_type(Type::U4, decoder);
    case 5: return parse_from_type(Type::U8, decoder);
    }
    UBLOX_UNREACHABLE();
    // TODO: this is unreachable, but GCC complains about it
    return CfgValue(Type::L, false);
}

CfgValue CfgValue::parse_from_type(Type type, Decoder& decoder) UBLOX_NOEXCEPT {
    switch (type) {
    case Type::L: return CfgValue::from_l(decoder.L());
    case Type::U1: return CfgValue::from_u1(decoder.U1());
    case Type::U2: return CfgValue::from_u2(decoder.U2());
    case Type::U4: return CfgValue::from_u4(decoder.U4());
    case Type::U8: return CfgValue::from_u8(decoder.U8());
    }
    UBLOX_UNREACHABLE();
    // TODO: this is unreachable, but GCC complains about it
    return CfgValue(Type::L, false);
}

}  // namespace ublox
}  // namespace receiver
