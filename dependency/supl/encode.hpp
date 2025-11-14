#pragma once
#include <supl/pos.hpp>
#include <supl/posinit.hpp>
#include <supl/session.hpp>
#include <supl/start.hpp>

#include <utility>

namespace supl {

class EncodedMessage {
public:
    EncodedMessage() : mBuffer(nullptr), mSize(0) {}
    explicit EncodedMessage(uint8_t* buffer, size_t size) : mBuffer(buffer), mSize(size) {}
    ~EncodedMessage() { delete[] mBuffer; }

    EncodedMessage(EncodedMessage const&)      = delete;
    EncodedMessage& operator=(EncodedMessage&) = delete;

    EncodedMessage(EncodedMessage&& other) noexcept : mBuffer(other.mBuffer), mSize(other.mSize) {
        other.mBuffer = nullptr;
        other.mSize   = 0;
    }
    EncodedMessage& operator=(EncodedMessage&& other) noexcept {
        if (this != &other) {
            delete[] mBuffer;
            mBuffer       = other.mBuffer;
            mSize         = other.mSize;
            other.mBuffer = nullptr;
            other.mSize   = 0;
        }
        return *this;
    }

    NODISCARD uint8_t const* data() const { return mBuffer; }
    NODISCARD size_t         size() const { return mSize; }

private:
    uint8_t* mBuffer;
    size_t   mSize;
};

EncodedMessage encode(Version version, Session::SET& set, Session::SLP& slp, const START& message);
EncodedMessage encode(Version version, Session::SET& set, Session::SLP& slp,
                      const POSINIT& message);
EncodedMessage encode(Version version, Session::SET& set, Session::SLP& slp, const POS& message);

}  // namespace supl
