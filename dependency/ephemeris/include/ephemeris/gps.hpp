#pragma once
#include <core/core.hpp>
#include <ephemeris/result.hpp>
#include <maths/float3.hpp>
#include <time/gps.hpp>

namespace ephemeris {

struct GpsEphemeris {
    uint8_t prn;

    uint16_t week_number;
    uint8_t  ca_or_p_on_l2;
    uint8_t  ura_index;
    uint8_t  sv_health;

    uint16_t lpp_iod;
    uint16_t iodc;
    uint8_t  iode;
    uint8_t  aodo;

    double toc;
    double toe;
    double tgd;

    double af2;
    double af1;
    double af0;

    double crc;
    double crs;
    double cuc;
    double cus;
    double cic;
    double cis;

    double e;
    double m0;
    double delta_n;
    double a;

    double i0;
    double omega0;
    double omega;
    double omega_dot;
    double idot;

    bool fit_interval_flag;
    bool l2_p_data_flag;

    NODISCARD bool is_valid(ts::Gps const& time) const NOEXCEPT;
    NODISCARD bool match(GpsEphemeris const& other) const NOEXCEPT {
        if(prn != other.prn) return false;
        if(week_number != other.week_number) return false;
        if(iode != other.iode) return false;
        if(iodc != other.iodc) return false;
        if(lpp_iod != other.lpp_iod) return false;
        if(std::abs(toe - other.toe) > 1e-3) return false;
        if(std::abs(toc - other.toc) > 1e-3) return false;
        if(fit_interval_flag != other.fit_interval_flag) return false;
        if(sv_health != other.sv_health) return false;
        return true;
    }
    NODISCARD bool compare(GpsEphemeris const& other) const NOEXCEPT {
        if(week_number < other.week_number) return true;
        if(week_number > other.week_number) return false;
        if(toe < other.toe) return true;
        if(toe > other.toe) return false;
        if(toc < other.toc) return true;
        if(toc > other.toc) return false;
        if(lpp_iod < other.lpp_iod) return true;
        if(lpp_iod > other.lpp_iod) return false;
        if(iode < other.iode) return true;
        if(iode > other.iode) return false;
        if(iodc < other.iodc) return true;
        if(iodc > other.iodc) return false;
        if(fit_interval_flag < other.fit_interval_flag) return true;
        if(fit_interval_flag > other.fit_interval_flag) return false;
        if(sv_health < other.sv_health) return true;
        if(sv_health > other.sv_health) return false;
        return false;
    }
    NODISCARD EphemerisResult compute(ts::Gps const& time) const NOEXCEPT;

    NODISCARD double calculate_elapsed_time(ts::Gps const& time, double reference) const NOEXCEPT;
    NODISCARD double calculate_elapsed_time_toe(ts::Gps const& time) const NOEXCEPT;
    NODISCARD double calculate_elapsed_time_toc(ts::Gps const& time) const NOEXCEPT;
    NODISCARD double calculate_clock_bias(ts::Gps const& time) const NOEXCEPT;
    NODISCARD double calculate_eccentric_anomaly(double t_k) const NOEXCEPT;
    NODISCARD double calculate_eccentric_anomaly_rate(double e_k) const NOEXCEPT;
    NODISCARD double calculate_relativistic_correction(Float3 const& position,
                                                       Float3 const& velocity) const NOEXCEPT;
    NODISCARD double calculate_group_delay() const NOEXCEPT;
};

}  // namespace ephemeris
