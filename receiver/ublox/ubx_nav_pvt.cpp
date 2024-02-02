#include "ubx_nav_pvt.hpp"
#include <stdio.h>
#include "decoder.hpp"

namespace receiver {
namespace ublox {

UbxNavPvt::UbxNavPvt(raw::NavPvt payload) UBLOX_NOEXCEPT : Message(CLASS_ID, MESSAGE_ID),
                                                           mPayload(std::move(payload)) {}

double UbxNavPvt::latitude() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.lat) * 1e-7;
}

double UbxNavPvt::longitude() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.lon) * 1e-7;
}

double UbxNavPvt::altitude() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.height) * 1e-3;
}

double UbxNavPvt::h_acc() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.h_acc) * 1e-3;
}

double UbxNavPvt::h_vel() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.g_speed) * 1e-3;
}

double UbxNavPvt::h_vel_acc() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.s_acc) * 1e-3;
}

double UbxNavPvt::head_mot() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.head_mot) * 1e-5;
}

double UbxNavPvt::v_acc() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.v_acc) * 1e-3;
}

double UbxNavPvt::v_vel() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.vel_d) * 1e-3;
}

double UbxNavPvt::v_vel_acc() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.s_acc) * 1e-3;
}

uint8_t UbxNavPvt::fix_type() const UBLOX_NOEXCEPT {
    return mPayload.fix_type;
}

uint8_t UbxNavPvt::carr_soln() const UBLOX_NOEXCEPT {
    return mPayload.flags.carr_soln;
}

uint8_t UbxNavPvt::num_sv() const UBLOX_NOEXCEPT {
    return mPayload.num_sv;
}

double UbxNavPvt::p_dop() const UBLOX_NOEXCEPT {
    return static_cast<double>(mPayload.p_dop) * 0.01;
}

time_t UbxNavPvt::timestamp() const UBLOX_NOEXCEPT {
    struct tm tm {};
    tm.tm_year  = mPayload.year - 1900;
    tm.tm_mon   = mPayload.month - 1;
    tm.tm_mday  = mPayload.day;
    tm.tm_hour  = mPayload.hour;
    tm.tm_min   = mPayload.min;
    tm.tm_sec   = mPayload.sec;
    tm.tm_isdst = 0;

    auto time = mktime(&tm);
    return time;
}

TAI_Time UbxNavPvt::tai_time() const UBLOX_NOEXCEPT {
    auto time     = timestamp();
    auto seconds  = static_cast<s64>(time);
    auto fraction = static_cast<f64>(mPayload.nano) * 1.0e-9;
    if (fraction < 0.0) {
        seconds -= 1;
        fraction += 1.0;
    }

    if (seconds < 0) {
        seconds  = 0;
        fraction = 0.0;
    }

    auto ts = Timestamp{seconds, fraction};
    return TAI_Time{UTC_Time{ts}};
}

bool UbxNavPvt::valid_time() const UBLOX_NOEXCEPT {
    return mPayload.valid.valid_date != 0 && mPayload.valid.valid_time != 0;
}

void UbxNavPvt::print() const UBLOX_NOEXCEPT {
    printf("[%02X %02X] UBX-NAV-PVT:\n", message_class(), message_id());
    printf("[.....]    i_tow: %u\n", mPayload.i_tow);
    printf("[.....]    year: %u\n", mPayload.year);
    printf("[.....]    month: %u\n", mPayload.month);
    printf("[.....]    day: %u\n", mPayload.day);
    printf("[.....]    hour: %u\n", mPayload.hour);
    printf("[.....]    min: %u\n", mPayload.min);
    printf("[.....]    sec: %u\n", mPayload.sec);
    printf("[.....]    valid:\n");
    printf("[.....]        valid_date: %u\n", mPayload.valid.valid_date);
    printf("[.....]        valid_time: %u\n", mPayload.valid.valid_time);
    printf("[.....]        fully_resolved: %u\n", mPayload.valid.fully_resolved);
    printf("[.....]        valid_mag: %u\n", mPayload.valid.valid_mag);
    printf("[.....]    t_acc: %u\n", mPayload.t_acc);
    printf("[.....]    nano: %d\n", mPayload.nano);
    printf("[.....]    fix_type: %u\n", mPayload.fix_type);
    printf("[.....]    flags:\n");
    printf("[.....]        gnss_fix_ok: %u\n", mPayload.flags.gnss_fix_ok);
    printf("[.....]        diff_soln: %u\n", mPayload.flags.diff_soln);
    printf("[.....]        psm_state: %u\n", mPayload.flags.psm_state);
    printf("[.....]        head_veh_valid: %u\n", mPayload.flags.head_veh_valid);
    printf("[.....]        carr_soln: %u\n", mPayload.flags.carr_soln);
    printf("[.....]    flags2:\n");
    printf("[.....]        confirmed_avai: %u\n", mPayload.flags2.confirmed_avai);
    printf("[.....]        confirmed_date: %u\n", mPayload.flags2.confirmed_date);
    printf("[.....]        confirmed_time: %u\n", mPayload.flags2.confirmed_time);
    printf("[.....]    num_sv: %u\n", mPayload.num_sv);
    printf("[.....]    lon: %f deg\n", longitude());
    printf("[.....]    lat: %f deg\n", latitude());
    printf("[.....]    height: %f m\n", altitude());
    printf("[.....]    h_msl: %d\n", mPayload.h_msl);
    printf("[.....]    h_acc: %u\n", mPayload.h_acc);
    printf("[.....]    v_acc: %u\n", mPayload.v_acc);
    printf("[.....]    vel_n: %d\n", mPayload.vel_n);
    printf("[.....]    vel_e: %d\n", mPayload.vel_e);
    printf("[.....]    vel_d: %d\n", mPayload.vel_d);
    printf("[.....]    g_speed: %d\n", mPayload.g_speed);
    printf("[.....]    head_mot: %d\n", mPayload.head_mot);
    printf("[.....]    s_acc: %u\n", mPayload.s_acc);
    printf("[.....]    head_acc: %u\n", mPayload.head_acc);
    printf("[.....]    p_dop: %u\n", mPayload.p_dop);
    printf("[.....]    flags3:\n");
    printf("[.....]        invalid_llh: %u\n", mPayload.flags3.invalid_llh);
    printf("[.....]        last_correction_arg: %u\n", mPayload.flags3.last_correction_arg);
    printf("[.....]    head_veh: %d\n", mPayload.head_veh);
    printf("[.....]    mag_dec: %d\n", mPayload.mag_dec);
    printf("[.....]    mag_acc: %u\n", mPayload.mag_acc);
}

std::unique_ptr<Message> UbxNavPvt::parse(Decoder& decoder) UBLOX_NOEXCEPT {
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
    payload.valid.valid_date     = (valid >> 0) & 0x01;
    payload.valid.valid_time     = (valid >> 1) & 0x01;
    payload.valid.fully_resolved = (valid >> 2) & 0x01;
    payload.valid.valid_mag      = (valid >> 3) & 0x01;

    payload.t_acc    = decoder.U4();
    payload.nano     = decoder.I4();
    payload.fix_type = decoder.U1();

    auto flags                   = decoder.X1();
    payload.flags.gnss_fix_ok    = (flags >> 0) & 0x01;
    payload.flags.diff_soln      = (flags >> 1) & 0x01;
    payload.flags.psm_state      = (flags >> 2) & 0x07;
    payload.flags.head_veh_valid = (flags >> 5) & 0x01;
    payload.flags.carr_soln      = (flags >> 6) & 0x03;

    auto flags2                   = decoder.X1();
    payload.flags2.confirmed_avai = (flags2 >> 5) & 0x01;
    payload.flags2.confirmed_date = (flags2 >> 6) & 0x01;
    payload.flags2.confirmed_time = (flags2 >> 7) & 0x01;

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
    payload.flags3.invalid_llh         = (flags3 >> 0) & 0x01;
    payload.flags3.last_correction_arg = (flags3 >> 1) & 0x0F;

    decoder.skip(4);
    payload.head_veh = decoder.I4();
    payload.mag_dec  = decoder.I2();
    payload.mag_acc  = decoder.U2();

    if (decoder.error()) {
        return nullptr;
    } else {
        return std::unique_ptr<Message>{new UbxNavPvt(std::move(payload))};
    }
}

}  // namespace ublox
}  // namespace receiver
