#pragma once
#include <lpp/initiator.hpp>
#include <lpp/types.hpp>

#include <string>

namespace lpp {

class PeriodicSessionHandle {
public:
    PeriodicSessionHandle() : mId(-1), mInitiator(Initiator::TargetDevice) {}
    LPP_EXPLICIT PeriodicSessionHandle(long id, Initiator initiator) : mId(id), mInitiator(initiator) {}

    LPP_NODISCARD long      id() const { return mId; }
    LPP_NODISCARD Initiator initiator() const { return mInitiator; }
    LPP_NODISCARD bool is_client_initiated() const { return mInitiator == Initiator::TargetDevice; }
    LPP_NODISCARD bool is_server_initiated() const {
        return mInitiator == Initiator::LocationServer;
    }
    LPP_NODISCARD bool is_valid() const { return mId >= 0 && mId <= 255; }
    LPP_NODISCARD bool operator==(const PeriodicSessionHandle& other) const {
        return mId == other.mId && mInitiator == other.mInitiator;
    }

    LPP_NODISCARD std::string to_string() const;

    LPP_NODISCARD static PeriodicSessionHandle invalid() { return PeriodicSessionHandle{}; }

private:
    long      mId;
    Initiator mInitiator;
};

}  // namespace lpp

template <>
struct std::hash<lpp::PeriodicSessionHandle> {
    std::size_t operator()(const lpp::PeriodicSessionHandle& k) const {
        std::size_t result = 17;
        result             = result * 31 + hash<long>()(k.id());
        result             = result * 31 + hash<lpp::Initiator>()(k.initiator());
        return result;
    }
};
