#pragma once
#include <core/core.hpp>
#include <ephemeris/result.hpp>
#include <maths/float3.hpp>
#include <time/bdt.hpp>

namespace ephemeris {

struct BdsEphemeris {
    uint8_t prn;

    uint16_t week_number;
    uint8_t  sv_health;
    uint16_t iode;
    uint16_t iodc;

    double toc;
    double toe;

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

    NODISCARD bool            is_valid(ts::Bdt const& time) const NOEXCEPT;
    NODISCARD EphemerisResult compute(ts::Bdt const& time) const NOEXCEPT;

    NODISCARD double calculate_elapsed_time(ts::Bdt const& time, double reference) const NOEXCEPT;
    NODISCARD double calculate_elapsed_time_toe(ts::Bdt const& time) const NOEXCEPT;
    NODISCARD double calculate_elapsed_time_toc(ts::Bdt const& time) const NOEXCEPT;
    NODISCARD double calculate_clock_bias(ts::Bdt const& time, double e_k) const NOEXCEPT;
    NODISCARD double calculate_eccentric_anomaly(double t_k) const NOEXCEPT;
    NODISCARD double calculate_eccentric_anomaly_rate(double e_k) const NOEXCEPT;
};

}  // namespace ephemeris