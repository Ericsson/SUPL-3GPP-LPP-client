#pragma once
#include <receiver/nmea/message.hpp>
#include <receiver/nmea/types.hpp>

#include <memory>
#include <utility/time.h>

namespace receiver {
namespace nmea {

enum class GpggaFixQuality {
    Invalid       = 0,
    GpsFix        = 1,
    DgpsFix       = 2,
    PpsFix        = 3,
    RtkFixed      = 4,
    RtkFloat      = 5,
    DeadReckoning = 6,
};

class GpggaMessage final : public Message {
public:
    ~GpggaMessage() override = default;

    void print() const NMEA_NOEXCEPT override;

    /// Get the time of day.
    NMEA_NODISCARD const TAI_Time& time_of_day() const NMEA_NOEXCEPT { return mTimeOfDay; }

    /// Get the latitude.
    NMEA_NODISCARD double latitude() const NMEA_NOEXCEPT { return mLatitude; }

    /// Get the longitude.
    NMEA_NODISCARD double longitude() const NMEA_NOEXCEPT { return mLongitude; }

    /// Get the fix quality.
    NMEA_NODISCARD GpggaFixQuality fix_quality() const NMEA_NOEXCEPT { return mFixQuality; }

    /// Get the number of satellites in view.
    NMEA_NODISCARD int satellites_in_view() const NMEA_NOEXCEPT { return mSatellitesInView; }

    NMEA_NODISCARD static std::unique_ptr<GpggaMessage> parse(const std::string& payload);

private:
    NMEA_EXPLICIT GpggaMessage() NMEA_NOEXCEPT;

    TAI_Time        mTimeOfDay;
    double          mLatitude;
    double          mLongitude;
    GpggaFixQuality mFixQuality;
    int             mSatellitesInView;
};

}  // namespace nmea
}  // namespace receiver
