#pragma once
#include <memory>
#include <cassert>

#include "utility/types.h"
#define OPTIONAL_DEBUG 0

template <typename T>
struct Optional {
public:
    Optional() { mInitialized = false; }

    Optional(const Optional<T>& other) {
#if OPTIONAL_DEBUG
        printf("Optional<%s> const&\n", typeid(T).name());
#endif
        if (other.initialized()) {
            mInitialized = true;
            new (data()) T(other.const_value());
        } else {
            mInitialized = false;
        }
    }

    Optional(Optional<T>&& other) {
#if OPTIONAL_DEBUG
        printf("Optional<%s> &&\n", typeid(T).name());
#endif
        if (other.initialized()) {
            mInitialized = true;
            new (data()) T(std::move(other.value()));
        } else {
            mInitialized = false;
        }
    }

    Optional(const T& value) {
#if OPTIONAL_DEBUG
        printf("Optional<%s> const& value\n", typeid(T).name());
#endif
        mInitialized = true;
        new (data()) T(value);
    }

    Optional(T&& value) {
#if OPTIONAL_DEBUG
        printf("Optional<%s> && value\n", typeid(T).name());
#endif
        mInitialized = true;
        new (data()) T(std::move(value));
    }

    Optional<T>& operator=(const Optional<T>& other) {
#if OPTIONAL_DEBUG
        printf("Optional<%s> copy operator=\n", typeid(T).name());
#endif
        Optional<T> tmp{other};
        this->swap(tmp);
        return *this;
    }

    Optional<T>& operator=(Optional<T>&& other) {
#if OPTIONAL_DEBUG
        printf("Optional<%s> move operator=\n", typeid(T).name());
#endif
        this->swap(other);
        return *this;
    }

#if 0
    Optional<T>& operator=(const T& value) {
        Optional<T> tmp{value};
        this->swap(tmp);
        return *this;
    }

    Optional<T>& operator=(T&& value) {
        Optional<T> tmp{value};
        this->swap(tmp);
        return *this;
    }
#endif

    ~Optional() { clear(); }

    void swap(Optional<T>& other) noexcept {
        if (!initialized() && other.initialized()) {
            new (data()) T(std::move(other.value()));
            mInitialized = true;
            other.clear();
        } else if (initialized() && !other.initialized()) {
            new (other.data()) T(std::move(value()));
            other.mInitialized = true;
            clear();
        } else if (initialized() && other.initialized()) {
            using std::swap;
            swap(value(), other.value());
        }
    }

    NO_DISCARD bool initialized() const { return mInitialized; }

    NO_DISCARD T* data() { return reinterpret_cast<T*>(mData); }

    NO_DISCARD const T* const_data() const {
        return reinterpret_cast<const T*>(mData);
    }

    NO_DISCARD T& value() {
        assert(initialized());
        return *data();
    }

    NO_DISCARD const T& const_value() const {
        assert(initialized());
        return *const_data();
    }

    T* operator->() { return &value(); }

    NO_DISCARD T& unsafe_value() { return *data(); }

    NO_DISCARD const T& or_value(const T& other) {
        if (initialized()) {
            return *data();
        } else {
            return other;
        }
    }

    NO_DISCARD T* addr_or_null() {
        if (initialized()) {
            return data();
        } else {
            return nullptr;
        }
    }

    void clear() {
        if (initialized()) {
            data()->~T();
        }

        mInitialized = false;
    }

    NO_DISCARD T& set_value() {
        if (initialized()) {
            return *data();
        } else {
            mInitialized = true;
            new (data()) T();
            return *data();
        }
    }

private:
    alignas(T) u8   mData[sizeof(T)];
    bool mInitialized;
};
