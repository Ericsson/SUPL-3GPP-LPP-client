#include "ubx_nav_pvt.hpp"
#include "decoder.hpp"
#include <stdio.h>

namespace receiver {
namespace ublox {

UbxNavPvt::UbxNavPvt(raw::NavPvt payload) UBLOX_NOEXCEPT : Message(0x01, 0x07), mPayload(payload) {}

double UbxNavPvt::latitude() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.lat) * 1e-7;
}

double UbxNavPvt::longitude() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.lon) * 1e-7;
}

double UbxNavPvt::altitude() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.height) * 1e-3;
}

void UbxNavPvt::print() const UBLOX_NOEXCEPT {
    printf("UBX-NAV-PVT: i_tow=%u, year=%u, month=%u, day=%u, hour=%u, min=%u, sec=%u, "
           "valid_date=%u, valid_time=%u, fully_resolved=%u, valid_mag=%u, t_acc=%u, nano=%d, "
           "fix_type=%u, gnss_fix_ok=%u, diff_soln=%u, psm_state=%u, head_veh_valid=%u, "
           "carr_soln=%u, confirmed_avai=%u, confirmed_date=%u, confirmed_time=%u, num_sv=%u, "
           "lon=%d, lat=%d, height=%d, h_msl=%d, h_acc=%u, v_acc=%u, vel_n=%d, vel_e=%d, vel_d=%d, "
           "g_speed=%d, head_mot=%d, s_acc=%u, head_acc=%u, p_dop=%u, invalid_llh=%u, "
           "last_correction_arg=%u, head_veh=%d, mag_dec=%d, mag_acc=%u\n",
           mPayload.i_tow, mPayload.year, mPayload.month, mPayload.day, mPayload.hour, mPayload.min,
           mPayload.sec, mPayload.valid.valid_date, mPayload.valid.valid_time,
           mPayload.valid.fully_resolved, mPayload.valid.valid_mag, mPayload.t_acc, mPayload.nano,
           mPayload.fix_type, mPayload.flags.gnss_fix_ok, mPayload.flags.diff_soln,
           mPayload.flags.psm_state, mPayload.flags.head_veh_valid, mPayload.flags.carr_soln,
           mPayload.flags2.confirmed_avai, mPayload.flags2.confirmed_date,
           mPayload.flags2.confirmed_time, mPayload.num_sv, mPayload.lon, mPayload.lat,
           mPayload.height, mPayload.h_msl, mPayload.h_acc, mPayload.v_acc, mPayload.vel_n,
           mPayload.vel_e, mPayload.vel_d, mPayload.g_speed, mPayload.head_mot, mPayload.s_acc,
           mPayload.head_acc, mPayload.p_dop, mPayload.flags3.invalid_llh,
           mPayload.flags3.last_correction_arg, mPayload.head_veh, mPayload.mag_dec,
           mPayload.mag_acc);
}

UbxNavPvt* UbxNavPvt::parse(Decoder& decoder) UBLOX_NOEXCEPT {
    if (decoder.remaining() < 92) {
        return nullptr;
    }

    auto payload  = raw::NavPvt{};
    payload.i_tow = decoder.U4();
    payload.year  = decoder.U2();
    payload.month = decoder.U1();
    payload.day   = decoder.U1();
    payload.hour  = decoder.U1();
    payload.min   = decoder.U1();
    payload.sec   = decoder.U1();

    auto valid                   = decoder.X1();
    payload.valid.valid_date     = valid & 0x01;
    payload.valid.valid_time     = valid & 0x02;
    payload.valid.fully_resolved = valid & 0x04;
    payload.valid.valid_mag      = valid & 0x08;

    payload.t_acc    = decoder.U4();
    payload.nano     = decoder.I4();
    payload.fix_type = decoder.U1();

    auto flags                   = decoder.X1();
    payload.flags.gnss_fix_ok    = flags & 0x01;
    payload.flags.diff_soln      = flags & 0x02;
    payload.flags.psm_state      = (flags >> 2) & 0x07;
    payload.flags.head_veh_valid = flags & 0x20;
    payload.flags.carr_soln      = (flags >> 6) & 0x03;

    auto flags2                   = decoder.X1();
    payload.flags2.confirmed_avai = flags2 & 0x20;
    payload.flags2.confirmed_date = flags2 & 0x40;
    payload.flags2.confirmed_time = flags2 & 0x80;

    payload.num_sv   = decoder.U1();
    payload.lon      = decoder.I4();
    payload.lat      = decoder.I4();
    payload.height   = decoder.I4();
    payload.h_msl    = decoder.I4();
    payload.h_acc    = decoder.U4();
    payload.v_acc    = decoder.U4();
    payload.vel_n    = decoder.I4();
    payload.vel_e    = decoder.I4();
    payload.vel_d    = decoder.I4();
    payload.g_speed  = decoder.I4();
    payload.head_mot = decoder.I4();
    payload.s_acc    = decoder.U4();
    payload.head_acc = decoder.U4();
    payload.p_dop    = decoder.U2();

    auto flags3                        = decoder.X1();
    payload.flags3.invalid_llh         = flags3 & 0x01;
    payload.flags3.last_correction_arg = (flags3 >> 1) & 0x0F;

    decoder.skip(4);
    payload.head_veh = decoder.I4();
    payload.mag_dec  = decoder.I2();
    payload.mag_acc  = decoder.U2();

    if (decoder.error()) {
        return nullptr;
    } else {
        return new UbxNavPvt(payload);
    }
}

}  // namespace ublox
}  // namespace receiver
