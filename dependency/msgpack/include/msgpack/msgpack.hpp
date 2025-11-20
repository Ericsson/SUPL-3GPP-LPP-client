#pragma once
#include <core/core.hpp>
#include <cstring>
#include <string>
#include <vector>

namespace msgpack {

class Packer {
public:
    EXPLICIT Packer(std::vector<uint8_t>& buffer) NOEXCEPT : mBuffer(buffer) {}

    void pack_nil() NOEXCEPT;
    void pack_bool(bool value) NOEXCEPT;
    void pack_int(int64_t value) NOEXCEPT;
    void pack_uint(uint64_t value) NOEXCEPT;
    void pack_float(float value) NOEXCEPT;
    void pack_double(double value) NOEXCEPT;
    void pack_str(char const* data, uint32_t length) NOEXCEPT;
    void pack_bin(uint8_t const* data, uint32_t length) NOEXCEPT;
    void pack_array_header(uint32_t size) NOEXCEPT;
    void pack_map_header(uint32_t size) NOEXCEPT;

private:
    void write_byte(uint8_t byte) NOEXCEPT;
    void write_bytes(uint8_t const* data, size_t length) NOEXCEPT;
    void write_uint16(uint16_t value) NOEXCEPT;
    void write_uint32(uint32_t value) NOEXCEPT;
    void write_uint64(uint64_t value) NOEXCEPT;

    std::vector<uint8_t>& mBuffer;
};

class Unpacker {
public:
    EXPLICIT Unpacker(uint8_t const* data, size_t length) NOEXCEPT : mData(data),
                                                                     mLength(length),
                                                                     mOffset(0) {}

    bool unpack_nil() NOEXCEPT;
    bool unpack_bool(bool& value) NOEXCEPT;
    bool unpack_int(int64_t& value) NOEXCEPT;
    bool unpack_uint(uint64_t& value) NOEXCEPT;
    bool unpack_float(float& value) NOEXCEPT;
    bool unpack_double(double& value) NOEXCEPT;
    bool unpack_str(char const*& data, uint32_t& length) NOEXCEPT;
    bool unpack_bin(uint8_t const*& data, uint32_t& length) NOEXCEPT;
    bool unpack_array_header(uint32_t& size) NOEXCEPT;
    bool unpack_map_header(uint32_t& size) NOEXCEPT;

    NODISCARD size_t offset() const NOEXCEPT { return mOffset; }
    NODISCARD bool   has_data() const NOEXCEPT { return mOffset < mLength; }

private:
    bool read_byte(uint8_t& byte) NOEXCEPT;
    bool read_bytes(uint8_t const*& data, size_t length) NOEXCEPT;
    bool read_uint16(uint16_t& value) NOEXCEPT;
    bool read_uint32(uint32_t& value) NOEXCEPT;
    bool read_uint64(uint64_t& value) NOEXCEPT;

    uint8_t const* mData;
    size_t         mLength;
    size_t         mOffset;
};

#define MSGPACK_DEFINE(...)                                                                        \
    void msgpack_pack(msgpack::Packer& packer) const NOEXCEPT {                                    \
        msgpack_pack_impl(packer, __VA_ARGS__);                                                    \
    }                                                                                              \
    bool msgpack_unpack(msgpack::Unpacker& unpacker) NOEXCEPT {                                    \
        return msgpack_unpack_impl(unpacker, __VA_ARGS__);                                         \
    }                                                                                              \
    template <typename... Args>                                                                    \
    void msgpack_pack_impl(msgpack::Packer& packer, Args&... args) const NOEXCEPT {                \
        packer.pack_array_header(sizeof...(args));                                                 \
        msgpack_pack_each(packer, args...);                                                        \
    }                                                                                              \
    template <typename... Args>                                                                    \
    bool msgpack_unpack_impl(msgpack::Unpacker& unpacker, Args&... args) NOEXCEPT {                \
        uint32_t size = 0;                                                                         \
        if (!unpacker.unpack_array_header(size)) return false;                                     \
        if (size != sizeof...(args)) return false;                                                 \
        return msgpack_unpack_each(unpacker, args...);                                             \
    }                                                                                              \
    template <typename T>                                                                          \
    void msgpack_pack_each(msgpack::Packer& packer, const T& value) const NOEXCEPT {               \
        msgpack::pack(packer, value);                                                              \
    }                                                                                              \
    template <typename T, typename... Args>                                                        \
    void msgpack_pack_each(msgpack::Packer& packer, const T& value, const Args&... args)           \
        const NOEXCEPT {                                                                           \
        msgpack::pack(packer, value);                                                              \
        msgpack_pack_each(packer, args...);                                                        \
    }                                                                                              \
    template <typename T>                                                                          \
    bool msgpack_unpack_each(msgpack::Unpacker& unpacker, T& value) NOEXCEPT {                     \
        return msgpack::unpack(unpacker, value);                                                   \
    }                                                                                              \
    template <typename T, typename... Args>                                                        \
    bool msgpack_unpack_each(msgpack::Unpacker& unpacker, T& value, Args&... args) NOEXCEPT {      \
        if (!msgpack::unpack(unpacker, value)) return false;                                       \
        return msgpack_unpack_each(unpacker, args...);                                             \
    }

inline void pack(Packer& packer, bool value) NOEXCEPT {
    packer.pack_bool(value);
}
inline void pack(Packer& packer, int8_t value) NOEXCEPT {
    packer.pack_int(value);
}
inline void pack(Packer& packer, int16_t value) NOEXCEPT {
    packer.pack_int(value);
}
inline void pack(Packer& packer, int32_t value) NOEXCEPT {
    packer.pack_int(value);
}
inline void pack(Packer& packer, int64_t value) NOEXCEPT {
    packer.pack_int(value);
}
inline void pack(Packer& packer, uint8_t value) NOEXCEPT {
    packer.pack_uint(value);
}
inline void pack(Packer& packer, uint16_t value) NOEXCEPT {
    packer.pack_uint(value);
}
inline void pack(Packer& packer, uint32_t value) NOEXCEPT {
    packer.pack_uint(value);
}
inline void pack(Packer& packer, uint64_t value) NOEXCEPT {
    packer.pack_uint(value);
}
inline void pack(Packer& packer, float value) NOEXCEPT {
    packer.pack_float(value);
}
inline void pack(Packer& packer, double value) NOEXCEPT {
    packer.pack_double(value);
}
inline void pack(Packer& packer, std::string const& value) NOEXCEPT {
    packer.pack_str(value.data(), static_cast<uint32_t>(value.size()));
}

inline bool unpack(Unpacker& unpacker, bool& value) NOEXCEPT {
    return unpacker.unpack_bool(value);
}
inline bool unpack(Unpacker& unpacker, int8_t& value) NOEXCEPT {
    int64_t tmp;
    if (!unpacker.unpack_int(tmp)) return false;
    value = static_cast<int8_t>(tmp);
    return true;
}
inline bool unpack(Unpacker& unpacker, int16_t& value) NOEXCEPT {
    int64_t tmp;
    if (!unpacker.unpack_int(tmp)) return false;
    value = static_cast<int16_t>(tmp);
    return true;
}
inline bool unpack(Unpacker& unpacker, int32_t& value) NOEXCEPT {
    int64_t tmp;
    if (!unpacker.unpack_int(tmp)) return false;
    value = static_cast<int32_t>(tmp);
    return true;
}
inline bool unpack(Unpacker& unpacker, int64_t& value) NOEXCEPT {
    return unpacker.unpack_int(value);
}
inline bool unpack(Unpacker& unpacker, uint8_t& value) NOEXCEPT {
    uint64_t tmp;
    if (!unpacker.unpack_uint(tmp)) return false;
    value = static_cast<uint8_t>(tmp);
    return true;
}
inline bool unpack(Unpacker& unpacker, uint16_t& value) NOEXCEPT {
    uint64_t tmp;
    if (!unpacker.unpack_uint(tmp)) return false;
    value = static_cast<uint16_t>(tmp);
    return true;
}
inline bool unpack(Unpacker& unpacker, uint32_t& value) NOEXCEPT {
    uint64_t tmp;
    if (!unpacker.unpack_uint(tmp)) return false;
    value = static_cast<uint32_t>(tmp);
    return true;
}
inline bool unpack(Unpacker& unpacker, uint64_t& value) NOEXCEPT {
    return unpacker.unpack_uint(value);
}
inline bool unpack(Unpacker& unpacker, float& value) NOEXCEPT {
    return unpacker.unpack_float(value);
}
inline bool unpack(Unpacker& unpacker, double& value) NOEXCEPT {
    return unpacker.unpack_double(value);
}
inline bool unpack(Unpacker& unpacker, std::string& value) NOEXCEPT {
    char const* data;
    uint32_t    length;
    if (!unpacker.unpack_str(data, length)) return false;
    value.assign(data, length);
    return true;
}

template <typename T>
void pack(Packer& packer, T const& value) NOEXCEPT {
    value.msgpack_pack(packer);
}

template <typename T>
bool unpack(Unpacker& unpacker, T& value) NOEXCEPT {
    return value.msgpack_unpack(unpacker);
}

}  // namespace msgpack
