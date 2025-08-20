#pragma once
#include <format/nmea/message.hpp>

#include <memory>

#include <time/tai.hpp>

namespace format {
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
          mGeoidSeparation(other.mGeoidSeparation),
          mAgeOfDifferentialCorrections(other.mAgeOfDifferentialCorrections) {}
    GgaMessage(GgaMessage&&)                 = delete;
    GgaMessage& operator=(GgaMessage const&) = delete;
    GgaMessage& operator=(GgaMessage&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;

    /// Get the time of day.
    NODISCARD const ts::Tai& time_of_day() const NOEXCEPT { return mTimeOfDay; }

    /// Get the latitude.
    NODISCARD double latitude() const NOEXCEPT { return mLatitude; }

    /// Get the longitude.
    NODISCARD double longitude() const NOEXCEPT { return mLongitude; }

    /// Get the fix quality.
    NODISCARD GgaFixQuality fix_quality() const NOEXCEPT { return mFixQuality; }

    /// Get the number of satellites in view.
    NODISCARD int satellites_in_view() const NOEXCEPT { return mSatellitesInView; }

    /// Get the horizontal dilution of precision.
    NODISCARD double h_dop() const NOEXCEPT { return mHdop; }

    /// Get the altitude in meters.
    NODISCARD double altitude() const NOEXCEPT { return mMsl + mGeoidSeparation; }

    /// Get the age of differential corrections.
    NODISCARD double age_of_differential_corrections() const NOEXCEPT {
        return mAgeOfDifferentialCorrections;
    }

    NODISCARD static std::unique_ptr<Message> parse(std::string prefix, std::string const& payload,
                                                    std::string checksum);

private:
    EXPLICIT GgaMessage(std::string prefix, std::string payload, std::string checksum) NOEXCEPT;

    ts::Tai       mTimeOfDay;
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
}  // namespace format
