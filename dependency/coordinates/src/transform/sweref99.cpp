// Transformation from ETRF2014 to SWEREF99 based on:
// "Transformationer mellan ITRF 2014 och SWEREF 99"
// Lantmäteriet, 2023-06-16
// https://www.lantmateriet.se/globalassets/geodata/gps-och-geodetisk-matning/transformations/transformation_itrf2014-sweref99.pdf

#include "coordinates/transform/sweref99.hpp"
#include "coordinates/ecef_enu.hpp"
#include "coordinates/ecef_llh.hpp"
#include "coordinates/model/nkgrf17vel.hpp"
#include "coordinates/transform_params.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(coord, sweref99);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(coord, sweref99)

namespace coordinates {

static constexpr Helmert7Params ETRF2014_TO_SWEREF99_2000_PARAMS{
    30.54,    // tx [mm]
    46.06,    // ty [mm]
    -79.44,   // tz [mm]
    1.41958,  // rx [mas]
    0.15132,  // ry [mas]
    1.50337,  // rz [mas]
    3.002     // s [ppb]
};

State Transform<ETRF2014, Sweref99>::apply(State const& state, double) {
    FUNCTION_SCOPE();
    auto tc           = state.epoch;
    auto xyz_etrf2014 = state.position;
    TRACEF("etrf2014(%.1f): %+.4f %+.4f %+.4f", tc, xyz_etrf2014.x(), xyz_etrf2014.y(),
           xyz_etrf2014.z());

    // Step 1: Get NKG_RF17vel velocity and change epoch to 2000.0
    auto llh = ecef_to_llh(Ecef<ETRF2014>{xyz_etrf2014});
    TRACEF("llh: %+.14f %+.14f", llh.latitude_deg(), llh.longitude_deg());
    auto vel_enu = Enu<ETRF2014>::from_any(nkgrf17vel::lookup(llh.to_any()));

    TRACEF("vel_enu: %+.4f %+.4f %+.4f [mm/year]", vel_enu.e() * 1000.0, vel_enu.n() * 1000.0,
           vel_enu.u() * 1000.0);

    auto R       = enu_rotation_matrix(llh.latitude(), llh.longitude());
    auto vel_xyz = R.transpose() * vel_enu.value;
    TRACEF("vel_xyz: %+.4f %+.4f %+.4f [mm/year]", vel_xyz.x() * 1000.0, vel_xyz.y() * 1000.0,
           vel_xyz.z() * 1000.0);

    auto dt2               = 2000.0 - tc;
    auto xyz_2000_etrf2014 = xyz_etrf2014 + dt2 * vel_xyz;

    TRACEF("etrf2014(2000.0): %+.4f %+.4f %+.4f", xyz_2000_etrf2014.x(), xyz_2000_etrf2014.y(),
           xyz_2000_etrf2014.z());

    // Step 2: Apply 7-parameter Helmert transformation (ETRF2014 to SWEREF99 at epoch 2000.0)
    auto xyz_2000_sweref99 = ETRF2014_TO_SWEREF99_2000_PARAMS.apply_position(xyz_2000_etrf2014);
    TRACEF("sweref99(2000.0): %+.4f %+.4f %+.4f", xyz_2000_sweref99.x(), xyz_2000_sweref99.y(),
           xyz_2000_sweref99.z());

    // Step 3: Change epoch to 1999.5
    auto dt4                 = 1999.5 - 2000.0;
    auto xyz_1999_5_sweref99 = xyz_2000_sweref99 + dt4 * vel_xyz;
    TRACEF("sweref99(1999.5): %+.4f %+.4f %+.4f", xyz_1999_5_sweref99.x(), xyz_1999_5_sweref99.y(),
           xyz_1999_5_sweref99.z());

    return {FrameId::Sweref99, 1999.5, xyz_1999_5_sweref99, vel_xyz};
}

State Transform<Sweref99, ETRF2014>::apply(State const& state, double target_epoch) {
    FUNCTION_SCOPE();
    auto tc           = state.epoch;
    auto xyz_sweref99 = state.position;
    TRACEF("sweref99(%.1f): %+.4f %+.4f %+.4f", tc, xyz_sweref99.x(), xyz_sweref99.y(),
           xyz_sweref99.z());

    auto llh     = ecef_to_llh(Ecef<Sweref99>{xyz_sweref99});
    auto vel_enu = Enu<Sweref99>::from_any(nkgrf17vel::lookup(llh.to_any()));
    auto R       = enu_rotation_matrix(llh.latitude(), llh.longitude());
    auto vel_xyz = R.transpose() * vel_enu.value;

    auto dt1               = 2000.0 - tc;
    auto xyz_2000_sweref99 = xyz_sweref99 + dt1 * vel_xyz;
    TRACEF("sweref99(2000.0): %+.4f %+.4f %+.4f", xyz_2000_sweref99.x(), xyz_2000_sweref99.y(),
           xyz_2000_sweref99.z());

    auto xyz_2000_etrf2014 =
        ETRF2014_TO_SWEREF99_2000_PARAMS.fast_inverse().apply_position(xyz_2000_sweref99);
    TRACEF("etrf2014(2000.0): %+.4f %+.4f %+.4f", xyz_2000_etrf2014.x(), xyz_2000_etrf2014.y(),
           xyz_2000_etrf2014.z());

    auto dt2                       = target_epoch - 2000.0;
    auto xyz_target_epoch_etrf2014 = xyz_2000_etrf2014 + dt2 * vel_xyz;
    TRACEF("etrf2014(%.1f): %+.4f %+.4f %+.4f", target_epoch, xyz_target_epoch_etrf2014.x(),
           xyz_target_epoch_etrf2014.y(), xyz_target_epoch_etrf2014.z());

    return {FrameId::ETRF2014, target_epoch, xyz_target_epoch_etrf2014, vel_xyz};
}

}  // namespace coordinates
