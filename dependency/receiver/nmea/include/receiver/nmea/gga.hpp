#pragma once
#include <receiver/nmea/message.hpp>
#include <receiver/nmea/types.hpp>

#include <memory>
#include <utility/time.h>

namespace receiver {
namespace nmea {

enum class GgaFixQuality {
    Invalid       = 0,
    GpsFix        = 1,
    DgpsFix       = 2,
    PpsFix        = 3,
    RtkFixed      = 4,
    RtkFloat      = 5,
    DeadReckoning = 6,
};

class GgaMessage final : public Message {
public:
    ~GgaMessage() override = default;

    GgaMessage(GgaMessage const& other)
        : Message(other), mTimeOfDay(other.mTimeOfDay), mLatitude(other.mLatitude),
          mLongitude(other.mLongitude), mFixQuality(other.mFixQuality),
          mSatellitesInView(other.mSatellitesInView), mHdop(other.mHdop), mMsl(other.mMsl),
          mGeoidSeparation(other.mGeoidSeparation), mAgeOfDifferentialCorrections(other.mAgeOfDifferentialCorrections) {}
    GgaMessage(GgaMessage&&)                 = delete;
    GgaMessage& operator=(GgaMessage const&) = delete;
    GgaMessage& operator=(GgaMessage&&)      = delete;

    void print() const NMEA_NOEXCEPT override;

    /// Get the time of day.
    NMEA_NODISCARD const TAI_Time& time_of_day() const NMEA_NOEXCEPT { return mTimeOfDay; }

    /// Get the latitude.
    NMEA_NODISCARD double latitude() const NMEA_NOEXCEPT { return mLatitude; }

    /// Get the longitude.
    NMEA_NODISCARD double longitude() const NMEA_NOEXCEPT { return mLongitude; }

    /// Get the fix quality.
    NMEA_NODISCARD GgaFixQuality fix_quality() const NMEA_NOEXCEPT { return mFixQuality; }

    /// Get the number of satellites in view.
    NMEA_NODISCARD int satellites_in_view() const NMEA_NOEXCEPT { return mSatellitesInView; }

    /// Get the horizontal dilution of precision.
    NMEA_NODISCARD double h_dop() const NMEA_NOEXCEPT { return mHdop; }

    /// Get the altitude in meters.
    NMEA_NODISCARD double altitude() const NMEA_NOEXCEPT { return mMsl + mGeoidSeparation; }

    /// Get the age of differential corrections.
    NMEA_NODISCARD double age_of_differential_corrections() const NMEA_NOEXCEPT {
        return mAgeOfDifferentialCorrections;
    }

    NMEA_NODISCARD static std::unique_ptr<Message>
    parse(std::string prefix, std::string const& payload, std::string checksum);

private:
    NMEA_EXPLICIT GgaMessage(std::string prefix, std::string payload,
                             std::string checksum) NMEA_NOEXCEPT;

    TAI_Time      mTimeOfDay;
    double        mLatitude;
    double        mLongitude;
    GgaFixQuality mFixQuality;
    int           mSatellitesInView;
    double        mHdop;
    double        mMsl;
    double        mGeoidSeparation;
    double        mAgeOfDifferentialCorrections;
};

}  // namespace nmea
}  // namespace receiver
