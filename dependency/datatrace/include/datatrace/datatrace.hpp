#pragma once
#include <core/core.hpp>
#include <maths/float3.hpp>
#include <time/tai.hpp>

#include <string>

#ifndef DATA_TRACING
#error "DATA_TRACING is not defined"
#endif

namespace datatrace {

template <typename T>
struct Option {
    T    value;
    bool valid;

    Option() : valid(false) {}
    Option(T const& new_value) : value(new_value), valid(true) {}
};

void initialize(std::string const& device, std::string const& server, int port,
                std::string const& username, std::string const& password);
void finalize();

struct Satellite {
    Option<Float3> position;
    Option<Float3> velocity;
    Option<double> elevation;
    Option<double> azimuth;
    Option<long>   iod;
    Option<Float3> eph_position;
};

struct Observation {
    Option<double> frequency;
    Option<double> geo_range;
    Option<double> eph_range;
    Option<double> eph_relativistic_correction;
    Option<double> sat_clock;
    Option<double> orbit;
    Option<double> clock;
    Option<double> code_bias;
    Option<double> phase_bias;
    Option<double> stec_grid;
    Option<double> stec_poly;
    Option<double> tropo_dry;
    Option<double> tropo_wet;
    Option<double> tropo_dry_mapping;
    Option<double> tropo_wet_mapping;
    Option<double> tropo_dry_height_correction;
    Option<double> tropo_wet_height_correction;
    Option<double> shapiro;
    Option<double> earth_solid_tides;
    Option<double> phase_windup;
    Option<double> phase_windup_velocity;
    Option<double> phase_windup_angle;
    Option<double> antenna_phase_variation;
    Option<double> code_range;
    Option<double> phase_range;
    Option<double> phase_range_rate;
    Option<double> carrier_to_noise_ratio;
    Option<double> phase_lock_time;
    Option<Float3> orbit_radial_axis;
    Option<Float3> orbit_cross_axis;
    Option<Float3> orbit_along_axis;
    Option<double> orbit_delta_t;
};

void report_satellite(ts::Tai const& time, std::string const& satellite, Satellite const& sat);
void report_observation(ts::Tai const& time, std::string const& satellite,
                        std::string const& signal, Observation const& obs);

void report_ssr_orbit_correction(ts::Tai const& time, std::string const& satellite,
                                 Option<Float3> delta, Option<Float3> dot_delta);
void report_ssr_clock_correction(ts::Tai const& time, std::string const& satellite,
                                 Option<double> c0, Option<double> c1, Option<double> c2);
void report_ssr_ionospheric_polynomial(ts::Tai const& time, std::string const& satellite,
                                       Option<double> c00, Option<double> c01, Option<double> c10,
                                       Option<double> c11, Option<double> reference_point_latitude,
                                       Option<double> reference_point_longitude);
void report_ssr_tropospheric_grid(ts::Tai const& time, int grid_point_id,
                                  Option<Float3> position_llh, Option<double> tropo_wet,
                                  Option<double> tropo_dry);
void report_ssr_ionospheric_grid(ts::Tai const& time, int grid_point_id,
                                 Option<Float3> position_llh, std::string const& satellite,
                                 Option<double> residual);

}  // namespace datatrace
