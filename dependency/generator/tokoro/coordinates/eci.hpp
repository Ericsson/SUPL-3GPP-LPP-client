#pragma once
#include <maths/float3.hpp>
#include <maths/mat3.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

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
 */
void eci_to_ecef_matrix(ts::Tai const& time, EciEarthParameters const& earth_params,
                        Mat3* transform_out, double* gmst_out);

}  // namespace tokoro
}  // namespace generator
