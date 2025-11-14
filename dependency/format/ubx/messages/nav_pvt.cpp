#include "messages/nav_pvt.hpp"
#include "decoder.hpp"

#include <cstdio>

#include <time/utc.hpp>

namespace format {
namespace ubx {

UbxNavPvt::UbxNavPvt(raw::NavPvt payload, std::vector<uint8_t> data) NOEXCEPT
    : Message(CLASS_ID, MESSAGE_ID, std::move(data)),
      mPayload(std::move(payload)) {}

double UbxNavPvt::latitude() const NOEXCEPT {
    return static_cast<double>(mPayload.lat) * 1e-7;
}

double UbxNavPvt::longitude() const NOEXCEPT {
    return static_cast<double>(mPayload.lon) * 1e-7;
}

double UbxNavPvt::altitude() const NOEXCEPT {
    return static_cast<double>(mPayload.height) * 1e-3;
}

double UbxNavPvt::h_acc() const NOEXCEPT {
    return static_cast<double>(mPayload.h_acc) * 1e-3;
}

double UbxNavPvt::h_vel() const NOEXCEPT {
    return static_cast<double>(mPayload.g_speed) * 1e-3;
}

double UbxNavPvt::h_vel_acc() const NOEXCEPT {
    return static_cast<double>(mPayload.s_acc) * 1e-3;
}

double UbxNavPvt::head_mot() const NOEXCEPT {
    return static_cast<double>(mPayload.head_mot) * 1e-5;
}

double UbxNavPvt::v_acc() const NOEXCEPT {
    return static_cast<double>(mPayload.v_acc) * 1e-3;
}

double UbxNavPvt::v_vel() const NOEXCEPT {
    return static_cast<double>(mPayload.vel_d) * 1e-3;
}

double UbxNavPvt::v_vel_acc() const NOEXCEPT {
    return static_cast<double>(mPayload.s_acc) * 1e-3;
}

uint8_t UbxNavPvt::fix_type() const NOEXCEPT {
    return mPayload.fix_type;
}

uint8_t UbxNavPvt::carr_soln() const NOEXCEPT {
    return mPayload.flags.carr_soln;
}

uint8_t UbxNavPvt::num_sv() const NOEXCEPT {
    return mPayload.num_sv;
}

double UbxNavPvt::p_dop() const NOEXCEPT {
    return static_cast<double>(mPayload.p_dop) * 0.01;
}

time_t UbxNavPvt::timestamp() const NOEXCEPT {
    struct tm tm{};
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

ts::Tai UbxNavPvt::tai_time() const NOEXCEPT {
    auto time     = timestamp();
    auto seconds  = static_cast<int64_t>(time);
    auto fraction = static_cast<double>(mPayload.nano) * 1.0e-9;
    if (fraction < 0.0) {
        seconds -= 1;
        fraction += 1.0;
    }

    if (seconds < 0) {
        seconds  = 0;
        fraction = 0.0;
    }

    auto ts = ts::Timestamp{seconds, fraction};
    return ts::Tai{ts::Utc{ts}};
}

bool UbxNavPvt::valid_time() const NOEXCEPT {
    return mPayload.valid.valid_date != 0 && mPayload.valid.valid_time != 0;
}

double UbxNavPvt::age_of_correction_data() const NOEXCEPT {
    if (mPayload.flags3.last_correction_arg == 0)
        return -1;
    else if (mPayload.flags3.last_correction_arg == 1)
        return 0;
    else if (mPayload.flags3.last_correction_arg == 2)
        return 1;
    else if (mPayload.flags3.last_correction_arg == 3)
        return 2;
    else if (mPayload.flags3.last_correction_arg == 4)
        return 5;
    else if (mPayload.flags3.last_correction_arg == 5)
        return 10;
    else if (mPayload.flags3.last_correction_arg == 6)
        return 15;
    else if (mPayload.flags3.last_correction_arg == 7)
        return 20;
    else if (mPayload.flags3.last_correction_arg == 8)
        return 30;
    else if (mPayload.flags3.last_correction_arg == 9)
        return 45;
    else if (mPayload.flags3.last_correction_arg == 10)
        return 60;
    else if (mPayload.flags3.last_correction_arg == 12)
        return 90;
    else
        return 120;
}

void UbxNavPvt::print() const NOEXCEPT {
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

std::unique_ptr<Message> UbxNavPvt::clone() const NOEXCEPT {
    return std::unique_ptr<Message>{new UbxNavPvt{*this}};
}

std::unique_ptr<Message> UbxNavPvt::parse(Decoder& decoder, std::vector<uint8_t> data) NOEXCEPT {
    if (decoder.remaining() < 92) {
        return nullptr;
    }

    auto payload  = raw::NavPvt{};
    payload.i_tow = decoder.u4();
    payload.year  = decoder.u2();
    payload.month = decoder.u1();
    payload.day   = decoder.u1();
    payload.hour  = decoder.u1();
    payload.min   = decoder.u1();
    payload.sec   = decoder.u1();

    auto valid                   = decoder.x1();
    payload.valid.valid_date     = (valid >> 0) & 0x01;
    payload.valid.valid_time     = (valid >> 1) & 0x01;
    payload.valid.fully_resolved = (valid >> 2) & 0x01;
    payload.valid.valid_mag      = (valid >> 3) & 0x01;

    payload.t_acc    = decoder.u4();
    payload.nano     = decoder.i4();
    payload.fix_type = decoder.u1();

    auto flags                   = decoder.x1();
    payload.flags.gnss_fix_ok    = (flags >> 0) & 0x01;
    payload.flags.diff_soln      = (flags >> 1) & 0x01;
    payload.flags.psm_state      = (flags >> 2) & 0x07;
    payload.flags.head_veh_valid = (flags >> 5) & 0x01;
    payload.flags.carr_soln      = (flags >> 6) & 0x03;

    auto flags2                   = decoder.x1();
    payload.flags2.confirmed_avai = (flags2 >> 5) & 0x01;
    payload.flags2.confirmed_date = (flags2 >> 6) & 0x01;
    payload.flags2.confirmed_time = (flags2 >> 7) & 0x01;

    payload.num_sv   = decoder.u1();
    payload.lon      = decoder.i4();
    payload.lat      = decoder.i4();
    payload.height   = decoder.i4();
    payload.h_msl    = decoder.i4();
    payload.h_acc    = decoder.u4();
    payload.v_acc    = decoder.u4();
    payload.vel_n    = decoder.i4();
    payload.vel_e    = decoder.i4();
    payload.vel_d    = decoder.i4();
    payload.g_speed  = decoder.i4();
    payload.head_mot = decoder.i4();
    payload.s_acc    = decoder.u4();
    payload.head_acc = decoder.u4();
    payload.p_dop    = decoder.u2();

    auto flags3                        = decoder.x1();
    payload.flags3.invalid_llh         = (flags3 >> 0) & 0x01;
    payload.flags3.last_correction_arg = (flags3 >> 1) & 0x0F;

    decoder.skip(4);
    payload.head_veh = decoder.i4();
    payload.mag_dec  = decoder.i2();
    payload.mag_acc  = decoder.u2();

    if (decoder.error()) {
        return nullptr;
    } else {
        return std::unique_ptr<Message>{new UbxNavPvt(std::move(payload), std::move(data))};
    }
}

}  // namespace ubx
}  // namespace format
