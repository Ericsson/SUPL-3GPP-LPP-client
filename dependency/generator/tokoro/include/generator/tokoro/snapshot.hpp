#pragma once
#include <core/core.hpp>
#include <maths/float3.hpp>
#include <msgpack/msgpack.hpp>
#include <msgpack/vector.hpp>
#include <time/tai.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

namespace generator {
namespace tokoro {

class ReferenceStation;

struct SnapshotEphemeris {
    std::vector<uint8_t> data;

    MSGPACK_DEFINE(data)
};

struct BaselineCorrectionPointSet {
    uint16_t set_id;
    int64_t  grid_point_count;
    double   reference_point_latitude;
    double   reference_point_longitude;
    double   step_of_latitude;
    double   step_of_longitude;
    int64_t  number_of_steps_latitude;
    int64_t  number_of_steps_longitude;
    uint64_t bitmask;

    MSGPACK_DEFINE(set_id, grid_point_count, reference_point_latitude, reference_point_longitude,
                   step_of_latitude, step_of_longitude, number_of_steps_latitude,
                   number_of_steps_longitude, bitmask)
};

struct SnapshotConfig {
    Float3 itrf_position;
    Float3 rtcm_position;
    bool   gps;
    bool   glo;
    bool   gal;
    bool   bds;
    bool   qzs;
    bool   shapiro_correction;
    bool   earth_solid_tides_correction;
    bool   phase_windup_correction;
    bool   antenna_phase_variation_correction;
    bool   tropospheric_height_correction;
    double elevation_mask;
    bool   phase_range_rate;
    bool   negative_phase_windup;
    bool   require_code_bias;
    bool   require_phase_bias;
    bool   require_tropo;
    bool   require_iono;
    bool   use_tropospheric_model;
    bool   use_ionospheric_height_correction;

    MSGPACK_DEFINE(itrf_position, rtcm_position, gps, glo, gal, bds, qzs, shapiro_correction,
                   earth_solid_tides_correction, phase_windup_correction,
                   antenna_phase_variation_correction, tropospheric_height_correction,
                   elevation_mask, phase_range_rate, negative_phase_windup, require_code_bias,
                   require_phase_bias, require_tropo, require_iono, use_tropospheric_model,
                   use_ionospheric_height_correction)
};

struct SnapshotOrbitCorrection {
    int32_t  gnss;
    int32_t  prn;
    double   radial;
    double   along_track;
    double   cross_track;
    double   dot_radial;
    double   dot_along_track;
    double   dot_cross_track;
    uint16_t iod;

    MSGPACK_DEFINE(gnss, prn, radial, along_track, cross_track, dot_radial, dot_along_track,
                   dot_cross_track, iod)
};

struct SnapshotClockCorrection {
    int32_t gnss;
    int32_t prn;
    double  c0;
    double  c1;
    double  c2;

    MSGPACK_DEFINE(gnss, prn, c0, c1, c2)
};

struct SnapshotCodeBias {
    int32_t gnss;
    int32_t prn;
    int32_t signal;
    double  bias;

    MSGPACK_DEFINE(gnss, prn, signal, bias)
};

struct SnapshotPhaseBias {
    int32_t gnss;
    int32_t prn;
    int32_t signal;
    double  bias;

    MSGPACK_DEFINE(gnss, prn, signal, bias)
};

struct SnapshotIonosphericPolynomial {
    int32_t gnss;
    int32_t prn;
    double  c00;
    double  c01;
    double  c10;
    double  c11;
    double  reference_point_latitude;
    double  reference_point_longitude;
    double  quality_indicator;
    bool    quality_indicator_valid;

    MSGPACK_DEFINE(gnss, prn, c00, c01, c10, c11, reference_point_latitude,
                   reference_point_longitude, quality_indicator, quality_indicator_valid)
};

struct SnapshotIonosphereResidual {
    int32_t gnss;
    int32_t prn;
    double  residual;

    MSGPACK_DEFINE(gnss, prn, residual)
};

struct SnapshotGridPoint {
    int64_t                                 latitude_index;
    int64_t                                 longitude_index;
    double                                  latitude;
    double                                  longitude;
    double                                  height;
    bool                                    has_troposphere;
    double                                  troposphere_wet;
    double                                  troposphere_dry;
    std::vector<SnapshotIonosphereResidual> ionosphere_residuals;

    MSGPACK_DEFINE(latitude_index, longitude_index, latitude, longitude, height, has_troposphere,
                   troposphere_wet, troposphere_dry, ionosphere_residuals)
};

struct SnapshotGridData {
    int32_t                        gnss;
    std::vector<SnapshotGridPoint> grid_points;

    MSGPACK_DEFINE(gnss, grid_points)
};

struct SnapshotObservation {
    int32_t gnss;
    int32_t prn;
    int32_t signal;
    double  pseudorange;
    double  carrier_phase;
    double  doppler;
    double  cnr;
    double  elevation;
    double  azimuth;

    MSGPACK_DEFINE(gnss, prn, signal, pseudorange, carrier_phase, doppler, cnr, elevation, azimuth)
};

struct SnapshotInput {
    ts::Tai                                    time;
    Float3                                     position;
    SnapshotConfig                             config;
    std::vector<SnapshotEphemeris>             ephemeris;
    std::vector<SnapshotOrbitCorrection>       orbit_corrections;
    std::vector<SnapshotClockCorrection>       clock_corrections;
    std::vector<SnapshotCodeBias>              code_biases;
    std::vector<SnapshotPhaseBias>             phase_biases;
    std::vector<SnapshotIonosphericPolynomial> ionospheric_polynomials;
    std::vector<SnapshotGridData>              grid_data;
    BaselineCorrectionPointSet                 correction_point_set;

    MSGPACK_DEFINE(time, position, config, ephemeris, orbit_corrections, clock_corrections,
                   code_biases, phase_biases, ionospheric_polynomials, grid_data,
                   correction_point_set)
};

struct SnapshotOutput {
    std::vector<SnapshotObservation> observations;

    MSGPACK_DEFINE(observations)
};

void extract_observations(std::shared_ptr<ReferenceStation> const& station, SnapshotOutput& output);

}  // namespace tokoro
}  // namespace generator
