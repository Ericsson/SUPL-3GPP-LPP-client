#include "cfg.hpp"
#include "decoder.hpp"
#include "encoder.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE_FORWARD_REF(ubx);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(ubx)

namespace format {
namespace ubx {

CfgValue CfgValue::from_l(bool value) NOEXCEPT {
    return CfgValue(Type::L, value);
}

CfgValue CfgValue::from_u1(uint8_t value) NOEXCEPT {
    return CfgValue(Type::U1, value);
}

CfgValue CfgValue::from_u2(uint16_t value) NOEXCEPT {
    return CfgValue(Type::U2, value);
}

CfgValue CfgValue::from_u4(uint32_t value) NOEXCEPT {
    return CfgValue(Type::U4, value);
}

CfgValue CfgValue::from_u8(uint64_t value) NOEXCEPT {
    return CfgValue(Type::U8, value);
}

CfgValue::CfgValue(Type type, bool value) NOEXCEPT : mType(type) {
    mValue.m_logical = value;
}

CfgValue::CfgValue(Type type, uint8_t value) NOEXCEPT : mType(type) {
    mValue.m_uint8 = value;
}

CfgValue::CfgValue(Type type, uint16_t value) NOEXCEPT : mType(type) {
    mValue.m_uint16 = value;
}

CfgValue::CfgValue(Type type, uint32_t value) NOEXCEPT : mType(type) {
    mValue.m_uint32 = value;
}

CfgValue::CfgValue(Type type, uint64_t value) NOEXCEPT : mType(type) {
    mValue.m_uint64 = value;
}

//
//
//

bool CfgValue::l() const NOEXCEPT {
    return mValue.m_logical;
}

uint8_t CfgValue::u1() const NOEXCEPT {
    return mValue.m_uint8;
}

uint16_t CfgValue::u2() const NOEXCEPT {
    return mValue.m_uint16;
}

uint32_t CfgValue::u4() const NOEXCEPT {
    return mValue.m_uint32;
}

uint64_t CfgValue::u8() const NOEXCEPT {
    return mValue.m_uint64;
}

uint32_t CfgValue::size() const NOEXCEPT {
    return size_from_type(mType);
}

void CfgValue::serialize(Encoder& encoder) const NOEXCEPT {
    switch (mType) {
    case Type::L: encoder.logical(mValue.m_logical); break;
    case Type::U1: encoder.u1(mValue.m_uint8); break;
    case Type::U2: encoder.u2(mValue.m_uint16); break;
    case Type::U4: encoder.u4(mValue.m_uint32); break;
    case Type::U8: encoder.u8(mValue.m_uint64); break;
    case Type::UNKNOWN: UNREACHABLE();
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
        break;
#endif
    }
}

//
//
//

CfgValue::Type CfgValue::type_from_key(CfgKey key) NOEXCEPT {
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

uint32_t CfgValue::size_from_type(Type type) NOEXCEPT {
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

CfgValue CfgValue::parse_from_key(CfgKey key, Decoder& decoder) NOEXCEPT {
    auto size_bits = (key & 0x70000000U) >> 28;
    switch (size_bits) {
    case 1: return parse_from_type(Type::L, decoder);
    case 2: return parse_from_type(Type::U1, decoder);
    case 3: return parse_from_type(Type::U2, decoder);
    case 4: return parse_from_type(Type::U4, decoder);
    case 5: return parse_from_type(Type::U8, decoder);
    }
    UNREACHABLE();
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    // TODO: this is unreachable, but GCC complains about it
    return CfgValue{};
#endif
}

CfgValue CfgValue::parse_from_type(Type type, Decoder& decoder) NOEXCEPT {
    switch (type) {
    case Type::L: return CfgValue::from_l(decoder.logical());
    case Type::U1: return CfgValue::from_u1(decoder.u1());
    case Type::U2: return CfgValue::from_u2(decoder.u2());
    case Type::U4: return CfgValue::from_u4(decoder.u4());
    case Type::U8: return CfgValue::from_u8(decoder.u8());
    case Type::UNKNOWN: UNREACHABLE();
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
        break;
#endif
    }
    UNREACHABLE();
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    // TODO: this is unreachable, but GCC complains about it
    return CfgValue{};
#endif
}

}  // namespace ubx
}  // namespace format
