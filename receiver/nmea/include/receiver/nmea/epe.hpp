#pragma once
#include <receiver/nmea/message.hpp>
#include <receiver/nmea/types.hpp>

#include <cmath>
#include <memory>
#include <utility/time.h>

namespace receiver {
namespace nmea {

// Message specification
// $PQTMEPE,<MsgVer>,<EPE_North>,<EPE_East>,<EPE_Down>,<EPE_2D>,<EPE_3D>*<Checksum>
class EpeMessage final : public Message {
public:
    ~EpeMessage() override = default;

    EpeMessage(EpeMessage const& other)
        : Message(other), mMsgVer(other.mMsgVer), mNorth(other.mNorth), mEast(other.mEast),
          mDown(other.mDown), m2D(other.m2D), m3D(other.m3D) {}
    EpeMessage(EpeMessage&&)                 = delete;
    EpeMessage& operator=(EpeMessage const&) = delete;
    EpeMessage& operator=(EpeMessage&&)      = delete;

    void print() const NMEA_NOEXCEPT override;

    /// ----- EPE source messages -----

    /// Get the estimated north error
    NMEA_NODISCARD double north() const NMEA_NOEXCEPT { return mNorth; }

    /// Get the estimated east error
    NMEA_NODISCARD double east() const NMEA_NOEXCEPT { return mEast; }

    /// Get the estimated down error
    NMEA_NODISCARD double down() const NMEA_NOEXCEPT { return mDown; }

    /// Get the estimated 2D position error
    NMEA_NODISCARD double epe_2d() const NMEA_NOEXCEPT { return m2D; }

    /// Get the estimated 3D position error
    NMEA_NODISCARD double epe_3d() const NMEA_NOEXCEPT { return m3D; }

    /// ----- GST conversions -----

    /// Get the horizontal position error.
    NMEA_NODISCARD double horizontal_position_error() const NMEA_NOEXCEPT { return m2D; }

    /// Get semi-major axis.
    NMEA_NODISCARD double semi_major() const NMEA_NOEXCEPT { return m2D / sqrt(2); }

    /// Get semi-minor axis.
    NMEA_NODISCARD double semi_minor() const NMEA_NOEXCEPT { return m2D / sqrt(2); }

    /// Get the orientation of the semi-major axis.
    NMEA_NODISCARD double orientation() const NMEA_NOEXCEPT { return 0; }

    /// Get the vertical position error.
    NMEA_NODISCARD double vertical_position_error() const NMEA_NOEXCEPT {
        return sqrt((m3D * m3D) - (m2D * m2D));
    }

    // TODO(ehedpon) Implement conversion from north, east and down error to latitude and longitude.
    // Also unclear if vertical error is sigma_v or if it needs to be converted from down

    NMEA_NODISCARD static std::unique_ptr<Message>
    parse(std::string prefix, std::string const& payload, std::string checksum);

private:
    NMEA_EXPLICIT EpeMessage(std::string prefix, std::string payload,
                             std::string checksum) NMEA_NOEXCEPT;

    double mMsgVer;
    double mNorth;
    double mEast;
    double mDown;
    double m2D;
    double m3D;
};

}  // namespace nmea
}  // namespace receiver
