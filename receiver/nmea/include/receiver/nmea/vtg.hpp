#pragma once
#include <receiver/nmea/message.hpp>
#include <receiver/nmea/types.hpp>

#include <memory>
#include <utility/time.h>

namespace receiver {
namespace nmea {

enum class ModeIndicator {
    Unknown,
    Autonomous   = 'A',
    Differential = 'D',
};

class VtgMessage final : public Message {
public:
    ~VtgMessage() override = default;

    void print() const NMEA_NOEXCEPT override;

    /// Get the true course over ground in degrees from true north.
    NMEA_NODISCARD double true_course_over_ground() const NMEA_NOEXCEPT {
        return mTrueCourseOverGround;
    }

    /// Get the speed over ground in meters per second.
    NMEA_NODISCARD double speed_over_ground() const NMEA_NOEXCEPT {
        return mSpeedOverGroundKmh / 3.6;
    }

    NMEA_NODISCARD static std::unique_ptr<Message> parse(std::string        prefix,
                                                         const std::string& payload);

private:
    NMEA_EXPLICIT VtgMessage(std::string prefix) NMEA_NOEXCEPT;

    double        mTrueCourseOverGround;
    double        mMagneticCourseOverGround;
    double        mSpeedOverGroundKnots;
    double        mSpeedOverGroundKmh;
    ModeIndicator mModeIndicator;
};

}  // namespace nmea
}  // namespace receiver
