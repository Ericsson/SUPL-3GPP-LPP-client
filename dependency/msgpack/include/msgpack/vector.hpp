#pragma once
#include <msgpack/msgpack.hpp>
#include <vector>

namespace msgpack {

template <typename T>
void pack(Packer& packer, std::vector<T> const& value) NOEXCEPT {
    packer.pack_array_header(static_cast<uint32_t>(value.size()));
    for (auto const& item : value) {
        pack(packer, item);
    }
}

template <typename T>
bool unpack(Unpacker& unpacker, std::vector<T>& value) NOEXCEPT {
    uint32_t size;
    if (!unpacker.unpack_array_header(size)) return false;

    value.clear();
    value.reserve(size);

    for (uint32_t i = 0; i < size; ++i) {
        T item;
        if (!unpack(unpacker, item)) return false;
        value.push_back(item);
    }
    return true;
}

}  // namespace msgpack
