#pragma once
#include <receiver/nmea/message.hpp>
#include <receiver/nmea/types.hpp>

#include <memory>
#include <utility/time.h>

namespace receiver {
namespace nmea {

enum class ModeIndicator {
    Unknown,
    Autonomous = 'A',
};

class VtgMessage final : public Message {
public:
    ~VtgMessage() override = default;

    void print() const NMEA_NOEXCEPT override;

    NMEA_NODISCARD static std::unique_ptr<VtgMessage> parse(std::string        prefix,
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
