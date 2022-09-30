#pragma once
#include "utility/types.h"
#include "utility/optional.h"

class SatelliteID {
public:
    SatelliteID() = default;

    NO_DISCARD static Optional<SatelliteID> from_lpp(GNSS_System, u32);

    NO_DISCARD static Optional<SatelliteID> from_msm(GNSS_System, u32);
    NO_DISCARD static Optional<SatelliteID> from_df009(GNSS_System, u32);
    NO_DISCARD static Optional<SatelliteID> from_df038(GNSS_System, u32);

    NO_DISCARD static Optional<SatelliteID> from_df068(GNSS_System, u32);
    NO_DISCARD static Optional<SatelliteID> from_df252(GNSS_System, u32);
    NO_DISCARD static Optional<SatelliteID> from_df466(GNSS_System, u32);

    NO_DISCARD long lpp_id() const;
    NO_DISCARD Optional<long> as_df009() const;
    NO_DISCARD Optional<long> as_df038() const;

    inline bool operator==(const SatelliteID& other) const {
        return mSystem == other.mSystem && mId == other.mId;
    }

    inline bool operator!=(const SatelliteID& other) const {
        return !(*this == other);
    }

private:
    explicit SatelliteID(GNSS_System system, s32 id)
        : mSystem(system), mId(id) {}

    GNSS_System mSystem{GNSS_System::UNKNOWN};
    s32         mId{-1};
};
