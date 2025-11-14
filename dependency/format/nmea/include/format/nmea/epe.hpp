#pragma once
#include <format/nmea/message.hpp>

#include <memory>

#include <time/tai.hpp>

namespace format {
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

    void      print() const NOEXCEPT override;
    NODISCARD std::unique_ptr<Message> clone() const NOEXCEPT override;

    /// ----- EPE source messages -----

    /// Get the estimated north error
    NODISCARD double north() const NOEXCEPT { return mNorth; }

    /// Get the estimated east error
    NODISCARD double east() const NOEXCEPT { return mEast; }

    /// Get the estimated down error
    NODISCARD double down() const NOEXCEPT { return mDown; }

    /// Get the estimated 2D position error
    NODISCARD double epe_2d() const NOEXCEPT { return m2D; }

    /// Get the estimated 3D position error
    NODISCARD double epe_3d() const NOEXCEPT { return m3D; }

    /// ----- GST conversions -----

    /// Get the horizontal position error.
    NODISCARD double horizontal_position_error() const NOEXCEPT { return m2D; }

    /// Get semi-major axis.
    NODISCARD double semi_major() const NOEXCEPT;

    /// Get semi-minor axis.
    NODISCARD double semi_minor() const NOEXCEPT;

    /// Get the orientation of the semi-major axis.
    NODISCARD double orientation() const NOEXCEPT { return 0; }

    /// Get the vertical position error.
    NODISCARD double vertical_position_error() const NOEXCEPT;

    // TODO(ehedpon) Implement conversion from north, east and down error to latitude and longitude.
    // Also unclear if vertical error is sigma_v or if it needs to be converted from down

    NODISCARD static std::unique_ptr<Message> parse(std::string prefix, std::string const& payload,
                                                    std::string checksum);

private:
    EXPLICIT EpeMessage(std::string prefix, std::string payload, std::string checksum) NOEXCEPT;

    double mMsgVer;
    double mNorth;
    double mEast;
    double mDown;
    double m2D;
    double m3D;
};

}  // namespace nmea
}  // namespace format
