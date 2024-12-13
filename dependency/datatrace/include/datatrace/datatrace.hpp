#pragma once
#include <core/core.hpp>
#include <maths/float3.hpp>
#include <time/tai.hpp>

#include <string>

#ifndef DATA_TRACING
#error "DATA_TRACING is not defined"
#endif

namespace datatrace {

void initialize(std::string const& device, std::string const& server, int port,
                std::string const& username, std::string const& password);
void finalize();

struct Satellite {
    Float3 position;
    Float3 velocity;
    double elevation;
    double azimuth;
    long   iod;
};

struct Observation {
    double frequency;
    double geo_range;
    double sat_clock;
    double orbit;
    double clock;
    double code_bias;
    double phase_bias;
    double stec_grid;
    double stec_poly;
    double tropo_dry;
    double tropo_wet;
    double shapiro;
    double earth_solid_tides;
    double phase_windup;
    double antenna_phase_variation;
    double code_range;
    double phase_range;
    double phase_range_rate;
    double carrier_to_noise_ratio;
    double phase_lock_time;
};

void report_satellite(ts::Tai const& time, std::string const& satellite, Satellite const& sat);
void report_observation(ts::Tai const& time, std::string const& satellite,
                        std::string const& signal, Observation const& obs);

}  // namespace datatrace
