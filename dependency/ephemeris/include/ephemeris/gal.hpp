#pragma once
#include <core/core.hpp>
#include <ephemeris/result.hpp>
#include <maths/float3.hpp>
#include <msgpack/msgpack.hpp>
#include <time/gst.hpp>

#include <cmath>

namespace ephemeris {

struct GalEphemeris {
    uint8_t  prn;
    uint16_t week_number;
    uint16_t lpp_iod;
    uint16_t iod_nav;

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

    NODISCARD bool is_valid(ts::Gst const& time) const NOEXCEPT;
    NODISCARD bool match(GalEphemeris const& other) const NOEXCEPT {
        if (prn != other.prn) return false;
        if (week_number != other.week_number) return false;
        if (iod_nav != other.iod_nav) return false;
        if (lpp_iod != other.lpp_iod) return false;
        if (std::fabs(toe - other.toe) > 1e-3) return false;
        if (std::fabs(toc - other.toc) > 1e-3) return false;
        return true;
    }
    NODISCARD bool compare(GalEphemeris const& other) const NOEXCEPT {
        if (week_number < other.week_number) return true;
        if (week_number > other.week_number) return false;
        if (toe < other.toe) return true;
        if (toe > other.toe) return false;
        if (toc < other.toc) return true;
        if (toc > other.toc) return false;
        if (lpp_iod < other.lpp_iod) return true;
        if (lpp_iod > other.lpp_iod) return false;
        if (iod_nav < other.iod_nav) return true;
        if (iod_nav > other.iod_nav) return false;
        return false;
    }
    NODISCARD EphemerisResult compute(ts::Gst const& time) const NOEXCEPT;

    NODISCARD double calculate_elapsed_time(ts::Gst const& time, double reference) const NOEXCEPT;
    NODISCARD double calculate_elapsed_time_toe(ts::Gst const& time) const NOEXCEPT;
    NODISCARD double calculate_elapsed_time_toc(ts::Gst const& time) const NOEXCEPT;
    NODISCARD double calculate_clock_bias(ts::Gst const& time) const NOEXCEPT;
    NODISCARD double calculate_eccentric_anomaly(double t_k) const NOEXCEPT;
    NODISCARD double calculate_eccentric_anomaly_rate(double e_k) const NOEXCEPT;
    NODISCARD double calculate_relativistic_correction(Float3 const& position,
                                                       Float3 const& velocity) const NOEXCEPT;
    NODISCARD double calculate_relativistic_correction_idc(double e_k) const NOEXCEPT;

    MSGPACK_DEFINE(prn, week_number, lpp_iod, iod_nav, toc, toe, af2, af1, af0, crc, crs, cuc, cus,
                   cic, cis, e, m0, delta_n, a, i0, omega0, omega, omega_dot, idot)
};

}  // namespace ephemeris

namespace streamline {
template <typename T>
struct TypeName;

template <>
struct TypeName<ephemeris::GalEphemeris> {
    static char const* name() { return "ephemeris::GalEphemeris"; }
};
}  // namespace streamline
