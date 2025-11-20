#include "msgpack.hpp"
#include <arpa/inet.h>
#include <cstring>

namespace msgpack {

void Packer::write_byte(uint8_t byte) NOEXCEPT {
    mBuffer.push_back(byte);
}

void Packer::write_bytes(uint8_t const* data, size_t length) NOEXCEPT {
    mBuffer.insert(mBuffer.end(), data, data + length);
}

void Packer::write_uint16(uint16_t value) NOEXCEPT {
    uint16_t be = htons(value);
    write_bytes(reinterpret_cast<uint8_t const*>(&be), 2);
}

void Packer::write_uint32(uint32_t value) NOEXCEPT {
    uint32_t be = htonl(value);
    write_bytes(reinterpret_cast<uint8_t const*>(&be), 4);
}

void Packer::write_uint64(uint64_t value) NOEXCEPT {
    uint32_t high = htonl(static_cast<uint32_t>(value >> 32));
    uint32_t low  = htonl(static_cast<uint32_t>(value & 0xFFFFFFFF));
    write_bytes(reinterpret_cast<uint8_t const*>(&high), 4);
    write_bytes(reinterpret_cast<uint8_t const*>(&low), 4);
}

void Packer::pack_nil() NOEXCEPT {
    write_byte(0xc0);
}

void Packer::pack_bool(bool value) NOEXCEPT {
    write_byte(value ? 0xc3 : 0xc2);
}

void Packer::pack_int(int64_t value) NOEXCEPT {
    if (value >= 0) {
        pack_uint(static_cast<uint64_t>(value));
        return;
    }

    if (value >= -32) {
        write_byte(static_cast<uint8_t>(value));
    } else if (value >= INT8_MIN) {
        write_byte(0xd0);
        write_byte(static_cast<uint8_t>(value));
    } else if (value >= INT16_MIN) {
        write_byte(0xd1);
        write_uint16(static_cast<uint16_t>(value));
    } else if (value >= INT32_MIN) {
        write_byte(0xd2);
        write_uint32(static_cast<uint32_t>(value));
    } else {
        write_byte(0xd3);
        write_uint64(static_cast<uint64_t>(value));
    }
}

void Packer::pack_uint(uint64_t value) NOEXCEPT {
    if (value <= 0x7f) {
        write_byte(static_cast<uint8_t>(value));
    } else if (value <= UINT8_MAX) {
        write_byte(0xcc);
        write_byte(static_cast<uint8_t>(value));
    } else if (value <= UINT16_MAX) {
        write_byte(0xcd);
        write_uint16(static_cast<uint16_t>(value));
    } else if (value <= UINT32_MAX) {
        write_byte(0xce);
        write_uint32(static_cast<uint32_t>(value));
    } else {
        write_byte(0xcf);
        write_uint64(value);
    }
}

void Packer::pack_float(float value) NOEXCEPT {
    write_byte(0xca);
    uint32_t bits;
    std::memcpy(&bits, &value, 4);
    write_uint32(bits);
}

void Packer::pack_double(double value) NOEXCEPT {
    write_byte(0xcb);
    uint64_t bits;
    std::memcpy(&bits, &value, 8);
    write_uint64(bits);
}

void Packer::pack_str(char const* data, uint32_t length) NOEXCEPT {
    if (length <= 31) {
        write_byte(0xa0 | static_cast<uint8_t>(length));
    } else if (length <= UINT8_MAX) {
        write_byte(0xd9);
        write_byte(static_cast<uint8_t>(length));
    } else if (length <= UINT16_MAX) {
        write_byte(0xda);
        write_uint16(static_cast<uint16_t>(length));
    } else {
        write_byte(0xdb);
        write_uint32(length);
    }
    write_bytes(reinterpret_cast<uint8_t const*>(data), length);
}

void Packer::pack_bin(uint8_t const* data, uint32_t length) NOEXCEPT {
    if (length <= UINT8_MAX) {
        write_byte(0xc4);
        write_byte(static_cast<uint8_t>(length));
    } else if (length <= UINT16_MAX) {
        write_byte(0xc5);
        write_uint16(static_cast<uint16_t>(length));
    } else {
        write_byte(0xc6);
        write_uint32(length);
    }
    write_bytes(data, length);
}

void Packer::pack_array_header(uint32_t size) NOEXCEPT {
    if (size <= 15) {
        write_byte(0x90 | static_cast<uint8_t>(size));
    } else if (size <= UINT16_MAX) {
        write_byte(0xdc);
        write_uint16(static_cast<uint16_t>(size));
    } else {
        write_byte(0xdd);
        write_uint32(size);
    }
}

void Packer::pack_map_header(uint32_t size) NOEXCEPT {
    if (size <= 15) {
        write_byte(0x80 | static_cast<uint8_t>(size));
    } else if (size <= UINT16_MAX) {
        write_byte(0xde);
        write_uint16(static_cast<uint16_t>(size));
    } else {
        write_byte(0xdf);
        write_uint32(size);
    }
}

bool Unpacker::read_byte(uint8_t& byte) NOEXCEPT {
    if (mOffset >= mLength) return false;
    byte = mData[mOffset++];
    return true;
}

bool Unpacker::read_bytes(uint8_t const*& data, size_t length) NOEXCEPT {
    if (mOffset + length > mLength) return false;
    data = mData + mOffset;
    mOffset += length;
    return true;
}

bool Unpacker::read_uint16(uint16_t& value) NOEXCEPT {
    uint8_t const* data;
    if (!read_bytes(data, 2)) return false;
    uint16_t be;
    std::memcpy(&be, data, 2);
    value = ntohs(be);
    return true;
}

bool Unpacker::read_uint32(uint32_t& value) NOEXCEPT {
    uint8_t const* data;
    if (!read_bytes(data, 4)) return false;
    uint32_t be;
    std::memcpy(&be, data, 4);
    value = ntohl(be);
    return true;
}

bool Unpacker::read_uint64(uint64_t& value) NOEXCEPT {
    uint32_t high, low;
    if (!read_uint32(high)) return false;
    if (!read_uint32(low)) return false;
    value = (static_cast<uint64_t>(high) << 32) | low;
    return true;
}

bool Unpacker::unpack_nil() NOEXCEPT {
    uint8_t byte;
    if (!read_byte(byte)) return false;
    return byte == 0xc0;
}

bool Unpacker::unpack_bool(bool& value) NOEXCEPT {
    uint8_t byte;
    if (!read_byte(byte)) return false;
    if (byte == 0xc2) {
        value = false;
        return true;
    }
    if (byte == 0xc3) {
        value = true;
        return true;
    }
    return false;
}

bool Unpacker::unpack_int(int64_t& value) NOEXCEPT {
    uint8_t byte;
    if (!read_byte(byte)) return false;

    if ((byte & 0x80) == 0) {
        value = byte;
        return true;
    }
    if ((byte & 0xe0) == 0xe0) {
        value = static_cast<int8_t>(byte);
        return true;
    }

    switch (byte) {
    case 0xcc: {
        uint8_t tmp;
        if (!read_byte(tmp)) return false;
        value = tmp;
        return true;
    }
    case 0xcd: {
        uint16_t tmp;
        if (!read_uint16(tmp)) return false;
        value = tmp;
        return true;
    }
    case 0xce: {
        uint32_t tmp;
        if (!read_uint32(tmp)) return false;
        value = tmp;
        return true;
    }
    case 0xcf: {
        uint64_t tmp;
        if (!read_uint64(tmp)) return false;
        value = static_cast<int64_t>(tmp);
        return true;
    }
    case 0xd0: {
        uint8_t tmp;
        if (!read_byte(tmp)) return false;
        value = static_cast<int8_t>(tmp);
        return true;
    }
    case 0xd1: {
        uint16_t tmp;
        if (!read_uint16(tmp)) return false;
        value = static_cast<int16_t>(tmp);
        return true;
    }
    case 0xd2: {
        uint32_t tmp;
        if (!read_uint32(tmp)) return false;
        value = static_cast<int32_t>(tmp);
        return true;
    }
    case 0xd3: {
        uint64_t tmp;
        if (!read_uint64(tmp)) return false;
        value = static_cast<int64_t>(tmp);
        return true;
    }
    default: return false;
    }
}

bool Unpacker::unpack_uint(uint64_t& value) NOEXCEPT {
    uint8_t byte;
    if (!read_byte(byte)) return false;

    if ((byte & 0x80) == 0) {
        value = byte;
        return true;
    }

    switch (byte) {
    case 0xcc: {
        uint8_t tmp;
        if (!read_byte(tmp)) return false;
        value = tmp;
        return true;
    }
    case 0xcd: {
        uint16_t tmp;
        if (!read_uint16(tmp)) return false;
        value = tmp;
        return true;
    }
    case 0xce: {
        uint32_t tmp;
        if (!read_uint32(tmp)) return false;
        value = tmp;
        return true;
    }
    case 0xcf: return read_uint64(value);
    default: return false;
    }
}

bool Unpacker::unpack_float(float& value) NOEXCEPT {
    uint8_t byte;
    if (!read_byte(byte)) return false;
    if (byte != 0xca) return false;

    uint32_t bits;
    if (!read_uint32(bits)) return false;
    std::memcpy(&value, &bits, 4);
    return true;
}

bool Unpacker::unpack_double(double& value) NOEXCEPT {
    uint8_t byte;
    if (!read_byte(byte)) return false;
    if (byte != 0xcb) return false;

    uint64_t bits;
    if (!read_uint64(bits)) return false;
    std::memcpy(&value, &bits, 8);
    return true;
}

bool Unpacker::unpack_str(char const*& data, uint32_t& length) NOEXCEPT {
    uint8_t byte;
    if (!read_byte(byte)) return false;

    if ((byte & 0xe0) == 0xa0) {
        length = byte & 0x1f;
    } else if (byte == 0xd9) {
        uint8_t len;
        if (!read_byte(len)) return false;
        length = len;
    } else if (byte == 0xda) {
        uint16_t len;
        if (!read_uint16(len)) return false;
        length = len;
    } else if (byte == 0xdb) {
        if (!read_uint32(length)) return false;
    } else {
        return false;
    }

    uint8_t const* bytes;
    if (!read_bytes(bytes, length)) return false;
    data = reinterpret_cast<char const*>(bytes);
    return true;
}

bool Unpacker::unpack_bin(uint8_t const*& data, uint32_t& length) NOEXCEPT {
    uint8_t byte;
    if (!read_byte(byte)) return false;

    if (byte == 0xc4) {
        uint8_t len;
        if (!read_byte(len)) return false;
        length = len;
    } else if (byte == 0xc5) {
        uint16_t len;
        if (!read_uint16(len)) return false;
        length = len;
    } else if (byte == 0xc6) {
        if (!read_uint32(length)) return false;
    } else {
        return false;
    }

    return read_bytes(data, length);
}

bool Unpacker::unpack_array_header(uint32_t& size) NOEXCEPT {
    uint8_t byte;
    if (!read_byte(byte)) return false;

    if ((byte & 0xf0) == 0x90) {
        size = byte & 0x0f;
        return true;
    }
    if (byte == 0xdc) {
        uint16_t tmp;
        if (!read_uint16(tmp)) return false;
        size = tmp;
        return true;
    }
    if (byte == 0xdd) {
        return read_uint32(size);
    }
    return false;
}

bool Unpacker::unpack_map_header(uint32_t& size) NOEXCEPT {
    uint8_t byte;
    if (!read_byte(byte)) return false;

    if ((byte & 0xf0) == 0x80) {
        size = byte & 0x0f;
        return true;
    }
    if (byte == 0xde) {
        uint16_t tmp;
        if (!read_uint16(tmp)) return false;
        size = tmp;
        return true;
    }
    if (byte == 0xdf) {
        return read_uint32(size);
    }
    return false;
}

}  // namespace msgpack
