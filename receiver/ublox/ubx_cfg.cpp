#include "ubx_cfg.hpp"
#include "decoder.hpp"
#include "encoder.hpp"

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

uint32_t CfgValue::size() const UBLOX_NOEXCEPT {
    return size_from_type(mType);
}

void CfgValue::serialize(Encoder& encoder) const UBLOX_NOEXCEPT {
    switch (mType) {
    case Type::L: encoder.L(mValue.mL); break;
    case Type::U1: encoder.U1(mValue.mU1); break;
    case Type::U2: encoder.U2(mValue.mU2); break;
    case Type::U4: encoder.U4(mValue.mU4); break;
    case Type::U8: encoder.U8(mValue.mU8); break;
    case Type::UNKNOWN: UBLOX_UNREACHABLE(); break;
    }
}

//
//
//

CfgValue::Type CfgValue::type_from_key(CfgKey key) UBLOX_NOEXCEPT {
    auto size_bits = (key & 0x70000000U) >> 28;
    switch (size_bits) {
    case 1: return Type::L;
    case 2: return Type::U1;
    case 3: return Type::U2;
    case 4: return Type::U4;
    case 5: return Type::U8;
    }
    return Type::UNKNOWN;
}

uint32_t CfgValue::size_from_type(Type type) UBLOX_NOEXCEPT {
    switch (type) {
    case Type::L: return 1;
    case Type::U1: return 1;
    case Type::U2: return 2;
    case Type::U4: return 4;
    case Type::U8: return 8;
    case Type::UNKNOWN: return 0;
    }
    return 0;
}

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
    return CfgValue{};
}

CfgValue CfgValue::parse_from_type(Type type, Decoder& decoder) UBLOX_NOEXCEPT {
    switch (type) {
    case Type::L: return CfgValue::from_l(decoder.L());
    case Type::U1: return CfgValue::from_u1(decoder.U1());
    case Type::U2: return CfgValue::from_u2(decoder.U2());
    case Type::U4: return CfgValue::from_u4(decoder.U4());
    case Type::U8: return CfgValue::from_u8(decoder.U8());
    case Type::UNKNOWN: UBLOX_UNREACHABLE(); break;
    }
    UBLOX_UNREACHABLE();
    // TODO: this is unreachable, but GCC complains about it
    return CfgValue{};
}

}  // namespace ublox
}  // namespace receiver
