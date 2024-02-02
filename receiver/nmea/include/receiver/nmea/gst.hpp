#pragma once
#include <receiver/nmea/message.hpp>
#include <receiver/nmea/types.hpp>

#include <cmath>
#include <memory>
#include <utility/time.h>

namespace receiver {
namespace nmea {

class GstMessage final : public Message {
public:
    ~GstMessage() override = default;

    GstMessage(const GstMessage& other)
        : Message(other), mRmsValue(other.mRmsValue), mSemiMajorError(other.mSemiMajorError),
          mSemiMinorError(other.mSemiMinorError),
          mOrientationOfSemiMajorError(other.mOrientationOfSemiMajorError),
          mLatitudeError(other.mLatitudeError), mLongitudeError(other.mLongitudeError),
          mAltitudeError(other.mAltitudeError) {}
    GstMessage(GstMessage&&)                 = delete;
    GstMessage& operator=(const GstMessage&) = delete;
    GstMessage& operator=(GstMessage&&)      = delete;

    void print() const NMEA_NOEXCEPT override;

    /// Get the horizontal position error.
    NMEA_NODISCARD double horizontal_position_error() const NMEA_NOEXCEPT {
        // HPE = sqrt(semiMajorError^2 + semiMinorError^2)
        return sqrt(mSemiMajorError * mSemiMajorError + mSemiMinorError * mSemiMinorError);
    }

    /// Get the vertical position error.
    NMEA_NODISCARD double vertical_position_error() const NMEA_NOEXCEPT { return mAltitudeError; }

    NMEA_NODISCARD static std::unique_ptr<Message>
    parse(std::string prefix, const std::string& payload, std::string checksum);

private:
    NMEA_EXPLICIT GstMessage(std::string prefix, std::string payload,
                             std::string checksum) NMEA_NOEXCEPT;

    double mRmsValue;
    double mSemiMajorError;
    double mSemiMinorError;
    double mOrientationOfSemiMajorError;
    double mLatitudeError;
    double mLongitudeError;
    double mAltitudeError;
};

}  // namespace nmea
}  // namespace receiver
