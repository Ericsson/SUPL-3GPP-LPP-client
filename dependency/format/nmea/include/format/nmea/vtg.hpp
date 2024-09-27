#pragma once
#include <format/nmea/message.hpp>

#include <memory>

namespace format {
namespace nmea {

enum class ModeIndicator {
    Unknown,
    Autonomous   = 'A',
    Differential = 'D',
};

class VtgMessage final : public Message {
public:
    ~VtgMessage() override = default;

    VtgMessage(VtgMessage const& other)
        : Message(other), mTrueCourseOverGround(other.mTrueCourseOverGround),
          mMagneticCourseOverGround(other.mMagneticCourseOverGround),
          mSpeedOverGroundKnots(other.mSpeedOverGroundKnots),
          mSpeedOverGroundKmh(other.mSpeedOverGroundKmh), mModeIndicator(other.mModeIndicator) {}
    VtgMessage(VtgMessage&&)                 = delete;
    VtgMessage& operator=(VtgMessage const&) = delete;
    VtgMessage& operator=(VtgMessage&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;

    /// Get the true course over ground in degrees from true north.
    NODISCARD double true_course_over_ground() const NOEXCEPT {
        return mTrueCourseOverGround;
    }

    /// Get the speed over ground in meters per second.
    NODISCARD double speed_over_ground() const NOEXCEPT {
        return mSpeedOverGroundKmh / 3.6;
    }

    NODISCARD static std::unique_ptr<Message>
    parse(std::string prefix, std::string const& payload, std::string checksum);

private:
    EXPLICIT VtgMessage(std::string prefix, std::string payload,
                             std::string checksum) NOEXCEPT;

    double        mTrueCourseOverGround;
    double        mMagneticCourseOverGround;
    double        mSpeedOverGroundKnots;
    double        mSpeedOverGroundKmh;
    ModeIndicator mModeIndicator;
};

}  // namespace nmea
}  // namespace receiver
