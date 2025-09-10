#pragma once
#include <lpp/initiator.hpp>

#include <string>

namespace lpp {

class PeriodicSessionHandle {
public:
    PeriodicSessionHandle() : mId(-1), mInitiator(Initiator::TargetDevice) {}
    EXPLICIT PeriodicSessionHandle(long id, Initiator initiator) : mId(id), mInitiator(initiator) {}

    NODISCARD long      id() const { return mId; }
    NODISCARD Initiator initiator() const { return mInitiator; }
    NODISCARD bool is_client_initiated() const { return mInitiator == Initiator::TargetDevice; }
    NODISCARD bool is_server_initiated() const { return mInitiator == Initiator::LocationServer; }
    NODISCARD bool is_valid() const { return mId >= 0 && mId <= 255; }
    NODISCARD bool operator==(PeriodicSessionHandle const& other) const {
        return mId == other.mId && mInitiator == other.mInitiator;
    }

    NODISCARD std::string to_string() const;

    NODISCARD static PeriodicSessionHandle invalid() { return PeriodicSessionHandle{}; }

private:
    long      mId;
    Initiator mInitiator;
};

}  // namespace lpp

namespace std {
template <>
struct hash<lpp::PeriodicSessionHandle> {
    std::size_t operator()(lpp::PeriodicSessionHandle const& k) const {
        std::size_t result = 17;
        result             = result * 31 + hash<long>()(k.id());
        result             = result * 31 + hash<lpp::Initiator>()(k.initiator());
        return result;
    }
};
}  // namespace std
