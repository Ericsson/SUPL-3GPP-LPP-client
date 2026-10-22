#pragma once
#include <maths/float3.hpp>
#include <maths/mat3.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

/// Celestial reference frame of the vectors given to eci_to_ecef_matrix.
enum class EciFrame {
    /// Mean equator and equinox of J2000.0 (precession from J2000 to date is applied).
    J2000,
    /// Mean equator and equinox of the date (already precessed, e.g. the analytic Sun/Moon
    /// ephemerides); only nutation, sidereal rotation and polar motion are applied.
    MeanOfDate,
};

/// Julian centuries of Terrestrial Time (TT = UTC + (TAI-UTC) + 32.184 s) since J2000.0, the
/// time argument of precession, nutation and the analytic Sun/Moon series. Earth rotation
/// (GMST) uses UT1 and is computed separately.
double terrestrial_time_century(ts::Tai const& time);

struct EciEarthParameters {
    double xp;
    double yp;
    double ut1_utc;  // UT1 - UTC
};

/**
 * @brief Compute the transformation matrix from ECI to ECEF coordinates.
 *
 * This function computes the transformation matrix to convert coordinates from the ECI frame to the
 * ECEF frame at a given time. It also optionally computes the Greenwich Mean Sidereal Time (GMST).
 *
 * @param time The time at which the transformation is computed, given as a `ts::Tai` object.
 * @param earth_params The Earth parameters required for the transformation, given as an
 * `EciEarthParameters` object.
 * @param transform_out Pointer to a `Mat3` object where the resulting transformation matrix will be
 * stored. If nullptr, the transformation matrix is not returned.
 * @param gmst_out Pointer to a double where the resulting GMST will be stored. If nullptr, the GMST
 * is not returned.
 * @param frame The frame of the vectors that will be transformed (J2000 by default).
 */
void eci_to_ecef_matrix(ts::Tai const& time, EciEarthParameters const& earth_params,
                        Mat3* transform_out, double* gmst_out, EciFrame frame = EciFrame::J2000);

}  // namespace tokoro
}  // namespace generator
