#pragma once
#include <stdint.h>

namespace helper {
template <typename T>
inline T* asn1_allocate(size_t count = 1) {
    return reinterpret_cast<T*>(calloc(count, sizeof(T)));
}
inline void asn1_free(void* ptr) {
    free(ptr);
}
}  // namespace helper
