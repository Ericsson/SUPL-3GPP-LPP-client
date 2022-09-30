#pragma once
#include <string>

#include "utility/types.h"
#include "utility/optional.h"

class SignalID {
public:
    SignalID() = default;

    NO_DISCARD static Optional<SignalID> from_rtcm_msm(GNSS_System system,
                                                       u32         id);
    NO_DISCARD static Optional<SignalID> from_rtcm_df380(GNSS_System system,
                                                         u32         id);
    NO_DISCARD static Optional<SignalID> from_rtcm_df382(GNSS_System system,
                                                         u32         id);
    NO_DISCARD static Optional<SignalID> from_rtcm_df467(GNSS_System system,
                                                         u32         id);
    NO_DISCARD static Optional<SignalID> from_lpp(GNSS_System system, u32 id);

    NO_DISCARD const std::string name() const;

    NO_DISCARD Optional<long> rtcm_msm_id() const;
    NO_DISCARD long           lpp_id() const;

    inline bool operator==(const SignalID& other) const {
        return system == other.system && id == other.id;
    }

    inline bool operator!=(const SignalID& other) const {
        return !(*this == other);
    }

private:
    explicit SignalID(GNSS_System system, s32 id) : system(system), id(id) {}

    GNSS_System system{GNSS_System::UNKNOWN};
    s32         id{-1};
};