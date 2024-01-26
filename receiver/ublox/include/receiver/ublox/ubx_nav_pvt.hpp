#pragma once
#include <memory>
#include <receiver/ublox/message.hpp>
#include <utility/time.h>

namespace receiver {
namespace ublox {

namespace raw {
struct NavPvt {
    /* 0x00 */ uint32_t i_tow;
    /* 0x04 */ uint16_t year;
    /* 0x06 */ uint8_t  month;
    /* 0x07 */ uint8_t  day;
    /* 0x08 */ uint8_t  hour;
    /* 0x09 */ uint8_t  min;
    /* 0x0A */ uint8_t  sec;
    /* 0x0B */ struct {
        /* bit 0 */ uint8_t valid_date;
        /* bit 1 */ uint8_t valid_time;
        /* bit 2 */ uint8_t fully_resolved;
        /* bit 3 */ uint8_t valid_mag;
    } valid;
    /* 0x0C */ uint32_t t_acc;
    /* 0x10 */ int32_t  nano;
    /* 0x14 */ uint8_t  fix_type;
    /* 0x15 */ struct {
        /* bit 0 */ uint8_t   gnss_fix_ok;
        /* bit 1 */ uint8_t   diff_soln;
        /* bit 2-4 */ uint8_t psm_state;
        /* bit 5 */ uint8_t   head_veh_valid;
        /* bit 6-7 */ uint8_t carr_soln;
    } flags;
    /* 0x16 */ struct {
        /* bit 5 */ uint8_t confirmed_avai;
        /* bit 6 */ uint8_t confirmed_date;
        /* bit 7 */ uint8_t confirmed_time;
    } flags2;
    /* 0x17 */ uint8_t  num_sv;
    /* 0x18 */ int32_t  lon;
    /* 0x1C */ int32_t  lat;
    /* 0x20 */ int32_t  height;
    /* 0x24 */ int32_t  h_msl;
    /* 0x28 */ uint32_t h_acc;
    /* 0x2C */ uint32_t v_acc;
    /* 0x30 */ int32_t  vel_n;
    /* 0x34 */ int32_t  vel_e;
    /* 0x38 */ int32_t  vel_d;
    /* 0x3C */ int32_t  g_speed;
    /* 0x40 */ int32_t  head_mot;
    /* 0x44 */ uint32_t s_acc;
    /* 0x48 */ uint32_t head_acc;
    /* 0x4C */ uint16_t p_dop;
    /* 0x4E */ struct {
        /* bit 0 */ uint8_t   invalid_llh;
        /* bit 1-4 */ uint8_t last_correction_arg;
    } flags3;
    /* 0x50 */ uint8_t  reserved0[4];
    /* 0x54 */ int32_t  head_veh;
    /* 0x58 */ int16_t  mag_dec;  // Only supported in ADR 4.10 and later.
    /* 0x5A */ uint16_t mag_acc;  // Only supported in ADR 4.10 and later.
};
}  // namespace raw

class Decoder;
class UbxNavPvt : public Message {
public:
    UBLOX_CONSTEXPR static uint8_t CLASS_ID   = 0x01;
    UBLOX_CONSTEXPR static uint8_t MESSAGE_ID = 0x07;

    UBLOX_EXPLICIT UbxNavPvt(raw::NavPvt payload) UBLOX_NOEXCEPT;
    ~UbxNavPvt() override = default;

    UbxNavPvt(const UbxNavPvt& other) : Message(other), mPayload(other.mPayload) {}
    UbxNavPvt(UbxNavPvt&&)                 = delete;
    UbxNavPvt& operator=(const UbxNavPvt&) = delete;
    UbxNavPvt& operator=(UbxNavPvt&&)      = delete;

    /// Returns the unprocessed raw payload.
    UBLOX_NODISCARD const raw::NavPvt& payload() const UBLOX_NOEXCEPT { return mPayload; }

    /// Latitude in degrees.
    UBLOX_NODISCARD double latitude() const UBLOX_NOEXCEPT;
    /// Longitude in degrees.
    UBLOX_NODISCARD double longitude() const UBLOX_NOEXCEPT;
    /// Altitude in meters above ellipsoid
    UBLOX_NODISCARD double altitude() const UBLOX_NOEXCEPT;

    /// Horizontal accuracy in meters (stddev).
    UBLOX_NODISCARD double h_acc() const UBLOX_NOEXCEPT;
    /// Horizontal speed in m/s.
    UBLOX_NODISCARD double h_vel() const UBLOX_NOEXCEPT;
    /// Horizontal speed accuracy in m/s.
    UBLOX_NODISCARD double h_vel_acc() const UBLOX_NOEXCEPT;
    /// Heading in degrees.
    UBLOX_NODISCARD double head_mot() const UBLOX_NOEXCEPT;

    /// Vertical accuracy in meters (stddev).
    UBLOX_NODISCARD double v_acc() const UBLOX_NOEXCEPT;
    /// Vertical speed in m/s.
    UBLOX_NODISCARD double v_vel() const UBLOX_NOEXCEPT;
    /// Vertical speed accuracy in m/s.
    UBLOX_NODISCARD double v_vel_acc() const UBLOX_NOEXCEPT;

    /// Fix type.
    UBLOX_NODISCARD uint8_t fix_type() const UBLOX_NOEXCEPT;
    /// Carrier solution type.
    UBLOX_NODISCARD uint8_t carr_soln() const UBLOX_NOEXCEPT;
    /// Number of satellites used in the navigation solution.
    UBLOX_NODISCARD uint8_t num_sv() const UBLOX_NOEXCEPT;
    /// Position dilution of precision.
    UBLOX_NODISCARD double p_dop() const UBLOX_NOEXCEPT;

    /// UTC timestamp of the navigation epoch. (as a UNIX timestamp)
    UBLOX_NODISCARD time_t timestamp() const UBLOX_NOEXCEPT;
    /// TAI timestamp of the navigation epoch.
    UBLOX_NODISCARD TAI_Time tai_time() const UBLOX_NOEXCEPT;

    /// UTC timestamp is valid.
    UBLOX_NODISCARD bool valid_time() const UBLOX_NOEXCEPT;

    void print() const UBLOX_NOEXCEPT override;

    UBLOX_NODISCARD static std::unique_ptr<Message> parse(Decoder& decoder) UBLOX_NOEXCEPT;

private:
    raw::NavPvt mPayload;
};

}  // namespace ublox
}  // namespace receiver
