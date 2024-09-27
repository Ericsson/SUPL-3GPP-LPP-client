#pragma once
#include <format/nmea/message.hpp>

#include <cmath>
#include <memory>

namespace format {
namespace nmea {

class GstMessage final : public Message {
public:
    ~GstMessage() override = default;

    GstMessage(GstMessage const& other)
        : Message(other), mRmsValue(other.mRmsValue), mSemiMajorError(other.mSemiMajorError),
          mSemiMinorError(other.mSemiMinorError),
          mOrientationOfSemiMajorError(other.mOrientationOfSemiMajorError),
          mLatitudeError(other.mLatitudeError), mLongitudeError(other.mLongitudeError),
          mAltitudeError(other.mAltitudeError) {}
    GstMessage(GstMessage&&)                 = delete;
    GstMessage& operator=(GstMessage const&) = delete;
    GstMessage& operator=(GstMessage&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;

    /// Get the horizontal position error.
    NODISCARD double horizontal_position_error() const NOEXCEPT {
        // HPE = sqrt(semiMajorError^2 + semiMinorError^2)
        return sqrt(mSemiMajorError * mSemiMajorError + mSemiMinorError * mSemiMinorError);
    }

    /// Get semi-major axis.
    NODISCARD double semi_major() const NOEXCEPT { return mSemiMajorError; }

    /// Get semi-minor axis.
    NODISCARD double semi_minor() const NOEXCEPT { return mSemiMinorError; }

    /// Get the orientation of the semi-major axis.
    NODISCARD double orientation() const NOEXCEPT { return mOrientationOfSemiMajorError; }

    /// Get the vertical position error.
    NODISCARD double vertical_position_error() const NOEXCEPT { return mAltitudeError; }

    NODISCARD static std::unique_ptr<Message>
    parse(std::string prefix, std::string const& payload, std::string checksum);

private:
    EXPLICIT GstMessage(std::string prefix, std::string payload,
                             std::string checksum) NOEXCEPT;

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
