#include "datatrace.hpp"

#include <iomanip>
#include <mosquitto.h>
#include <sstream>

#include <loglet/loglet.hpp>
#include <time/utc.hpp>
#define LOGLET_CURRENT_MODULE "datatrace"

namespace datatrace {

static mosquitto*  gMosq        = nullptr;
static bool        gInitialized = false;
static bool        gEnabled     = false;
static std::string gDevice;
static std::string gServer;
static int         gPort;
static std::string gUsername;
static std::string gPassword;

void initialize(std::string const& device, std::string const& server, int port,
                std::string const& username, std::string const& password) {
    FUNCTION_SCOPE();
    ::mosquitto_lib_init();
    VERBOSEF("::mosquitto_lib_init()");

    gDevice      = device;
    gServer      = server;
    gPort        = port;
    gUsername    = username;
    gPassword    = password;
    gInitialized = false;
    gEnabled     = true;
}

void start() {
    if (gInitialized) return;
    if (!gEnabled) return;

    ASSERT(gMosq == nullptr, "mosq is not null");
    gMosq = ::mosquitto_new(nullptr, true, nullptr);
    VERBOSEF("::mosquitto_new(nullptr, true, nullptr) = %p", gMosq);
    if (gMosq == nullptr) {
        ERRORF("failed to create mosquitto instance");
        return;
    }

    auto result = ::mosquitto_max_inflight_messages_set(gMosq, 4096);
    VERBOSEF("::mosquitto_max_inflight_messages_set(%p, 4096) = %d", gMosq, result);
    if (result != MOSQ_ERR_SUCCESS) {
        WARNF("failed to set max inflight messages: %s", mosquitto_strerror(result));
    }

    result = ::mosquitto_username_pw_set(gMosq, gUsername.c_str(), gPassword.c_str());
    VERBOSEF("::mosquitto_username_pw_set(%p, %s, %s) = %d", gMosq, gUsername.c_str(),
             gPassword.c_str(), result);
    if (result != MOSQ_ERR_SUCCESS) {
        ERRORF("failed to set username and password: %s", mosquitto_strerror(result));
        mosquitto_destroy(gMosq);
        return;
    }

    result = ::mosquitto_connect(gMosq, gServer.c_str(), gPort, 60);
    VERBOSEF("::mosquitto_connect(%p, %s, %d, 60) = %d", gMosq, gServer.c_str(), gPort, result);
    if (result != MOSQ_ERR_SUCCESS) {
        ERRORF("failed to connect to mosquitto server: %s", mosquitto_strerror(result));
        mosquitto_destroy(gMosq);
        return;
    }

    result = ::mosquitto_loop_start(gMosq);
    VERBOSEF("::mosquitto_loop_start(%p) = %d", gMosq, result);
    if (result != MOSQ_ERR_SUCCESS) {
        ERRORF("failed to start mosquitto loop: %s", mosquitto_strerror(result));
        mosquitto_destroy(gMosq);
        return;
    }

    DEBUGF("mosquitto started");
    gInitialized = true;
}

void finalize() {
    FUNCTION_SCOPE();
    if (!gEnabled) return;
    if (gInitialized) {
        ASSERT(gMosq != nullptr, "mosq is null");

        ::mosquitto_lib_cleanup();
        VERBOSEF("::mosquitto_lib_cleanup()");
        gInitialized = false;
    }
}

void replace_all(std::string& str, std::string const& from, std::string const& to) {
    if (from.empty()) return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

void publish(std::string const& topic, std::string const& ss_data) {
    if (!gEnabled) return;
    if (!gInitialized) start();
    if (!gInitialized) {
        WARNF("failed to initialize mosquitto");
        return;
    }

    ASSERT(gMosq != nullptr, "mosq is null");

    int count = 0;
    while (count < 3) {
        auto ss_size = static_cast<int>(ss_data.size());
        auto result =
            ::mosquitto_publish(gMosq, nullptr, topic.c_str(), ss_size, ss_data.c_str(), 1, false);
        VERBOSEF("::mosquitto_publish(%p, nullptr, \"%s\", %zu, ..., 0, false) = %d", gMosq,
                 topic.c_str(), ss_size, result);
        if (result != MOSQ_ERR_SUCCESS) {
            DEBUGF("failed to publish (attempt %d): %s", count, mosquitto_strerror(result));
        } else {
            break;
        }
        count++;
    }
}

void report_observation(ts::Tai const& time, std::string const& satellite,
                        std::string const& signal, Observation const& obs) {
    FUNCTION_SCOPE();

    auto signal_copy = signal;
    replace_all(signal_copy, "+", "");
    replace_all(signal_copy, "/", "");
    auto topic    = "datatrace/obs/" + gDevice + "/" + satellite + "/" + signal_copy + "/v1";
    auto utc_time = ts::Utc{time};

    std::stringstream ss;
    ss << std::setprecision(14);
    ss << "{";
    ss << "\"time\":\"" << utc_time.rfc3339() << "\"";
    if (obs.frequency.valid) ss << ",\"freq\":" << obs.frequency.value;
    if (obs.geo_range.valid) ss << ",\"geo_range\":" << obs.geo_range.value;
    if (obs.eph_range.valid) ss << ",\"eph_range\":" << obs.eph_range.value;
    if (obs.eph_relativistic_correction.valid)
        ss << ",\"eph_relativistic_correction\":" << obs.eph_relativistic_correction.value;
    if (obs.sat_clock.valid) ss << ",\"sat_clock\":" << obs.sat_clock.value;
    if (obs.clock.valid) ss << ",\"clock\":" << obs.clock.value;
    if (obs.orbit.valid) ss << ",\"orbit\":" << obs.orbit.value;
    if (obs.code_bias.valid) ss << ",\"code_bias\":" << obs.code_bias.value;
    if (obs.phase_bias.valid) ss << ",\"phase_bias\":" << obs.phase_bias.value;
    if (obs.stec.valid) ss << ",\"stec\":" << obs.stec.value;
    if (obs.stec_grid.valid) ss << ",\"stec_grid\":" << obs.stec_grid.value;
    if (obs.stec_poly.valid) ss << ",\"stec_poly\":" << obs.stec_poly.value;
    if (obs.tropo.valid) ss << ",\"tropo\":" << obs.tropo.value;
    if (obs.tropo_dry.valid) ss << ",\"tropo_dry\":" << obs.tropo_dry.value;
    if (obs.tropo_wet.valid) ss << ",\"tropo_wet\":" << obs.tropo_wet.value;
    if (obs.tropo_dry_mapping.valid) ss << ",\"tropo_dry_mapping\":" << obs.tropo_dry_mapping.value;
    if (obs.tropo_wet_mapping.valid) ss << ",\"tropo_wet_mapping\":" << obs.tropo_wet_mapping.value;
    if (obs.tropo_dry_height_correction.valid)
        ss << ",\"tropo_dry_height_correction\":" << obs.tropo_dry_height_correction.value;
    if (obs.tropo_wet_height_correction.valid)
        ss << ",\"tropo_wet_height_correction\":" << obs.tropo_wet_height_correction.value;
    if (obs.tropo_model_dry.valid) ss << ",\"tropo_model_dry\":" << obs.tropo_model_dry.value;
    if (obs.tropo_model_wet.valid) ss << ",\"tropo_model_wet\":" << obs.tropo_model_wet.value;
    if (obs.shapiro.valid) ss << ",\"shapiro\":" << obs.shapiro.value;
    if (obs.earth_solid_tides.valid) ss << ",\"est\":" << obs.earth_solid_tides.value;
    if (obs.phase_windup.valid) ss << ",\"pw\":" << obs.phase_windup.value;
    if (obs.phase_windup_velocity.valid)
        ss << ",\"pw_velocity\":" << obs.phase_windup_velocity.value;
    if (obs.phase_windup_angle.valid) ss << ",\"pw_angle\":" << obs.phase_windup_angle.value;
    if (obs.antenna_phase_variation.valid) ss << ",\"apv\":" << obs.antenna_phase_variation.value;
    if (obs.code_range.valid) ss << ",\"code_range\":" << obs.code_range.value;
    if (obs.phase_range.valid) ss << ",\"phase_range\":" << obs.phase_range.value;
    if (obs.phase_range_rate.valid) ss << ",\"phase_range_rate\":" << obs.phase_range_rate.value;
    if (obs.carrier_to_noise_ratio.valid)
        ss << ",\"carrier_to_noise_ratio\":" << obs.carrier_to_noise_ratio.value;
    if (obs.phase_lock_time.valid) ss << ",\"phase_lock_time\":" << obs.phase_lock_time.value;

    if (obs.orbit_radial_axis.valid) {
        ss << ",\"orbit_radial_axis_x\":" << obs.orbit_radial_axis.value.x;
        ss << ",\"orbit_radial_axis_y\":" << obs.orbit_radial_axis.value.y;
        ss << ",\"orbit_radial_axis_z\":" << obs.orbit_radial_axis.value.z;
    }
    if (obs.orbit_cross_axis.valid) {
        ss << ",\"orbit_cross_axis_x\":" << obs.orbit_cross_axis.value.x;
        ss << ",\"orbit_cross_axis_y\":" << obs.orbit_cross_axis.value.y;
        ss << ",\"orbit_cross_axis_z\":" << obs.orbit_cross_axis.value.z;
    }
    if (obs.orbit_along_axis.valid) {
        ss << ",\"orbit_along_axis_x\":" << obs.orbit_along_axis.value.x;
        ss << ",\"orbit_along_axis_y\":" << obs.orbit_along_axis.value.y;
        ss << ",\"orbit_along_axis_z\":" << obs.orbit_along_axis.value.z;
    }
    if (obs.orbit_delta_t.valid) ss << ",\"orbit_delta_t\":" << obs.orbit_delta_t.value;
    if (obs.eph_iod.valid) ss << ",\"eph_iod\":" << obs.eph_iod.value;
    if (obs.vtec_mapping.valid) ss << ",\"vtec_mapping\":" << obs.vtec_mapping.value;
    if (obs.stec_height_correction.valid)
        ss << ",\"stec_height_correction\":" << obs.stec_height_correction.value;
    ss << "}";

    publish(topic, ss.str());
}

void report_satellite(ts::Tai const& time, std::string const& satellite, Satellite const& sat) {
    FUNCTION_SCOPE();

    auto topic    = "datatrace/sat/" + gDevice + "/" + satellite + "/v1";
    auto utc_time = ts::Utc{time};

    std::stringstream ss;
    ss << std::setprecision(14);
    ss << "{";
    ss << "\"time\":\"" << utc_time.rfc3339() << "\"";
    if (sat.position.valid) {
        ss << ",\"pos_x\":" << sat.position.value.x;
        ss << ",\"pos_y\":" << sat.position.value.y;
        ss << ",\"pos_z\":" << sat.position.value.z;
    }
    if (sat.velocity.valid) {
        ss << ",\"vel_x\":" << sat.velocity.value.x;
        ss << ",\"vel_y\":" << sat.velocity.value.y;
        ss << ",\"vel_z\":" << sat.velocity.value.z;
    }
    if (sat.elevation.valid) ss << ",\"elevation\":" << sat.elevation.value;
    if (sat.azimuth.valid) ss << ",\"azimuth\":" << sat.azimuth.value;
    if (sat.iod.valid) ss << ",\"iod\":" << sat.iod.value;
    if (sat.eph_position.valid) {
        ss << ",\"eph_pos_x\":" << sat.eph_position.value.x;
        ss << ",\"eph_pos_y\":" << sat.eph_position.value.y;
        ss << ",\"eph_pos_z\":" << sat.eph_position.value.z;
    }
    ss << "}";

    publish(topic, ss.str());
}

void report_ssr_orbit_correction(ts::Tai const& time, std::string const& satellite,
                                 Option<Float3> delta, Option<Float3> dot_delta,
                                 Option<long> ssr_iod, Option<long> eph_iod) {
    FUNCTION_SCOPE();

    auto topic    = "datatrace/ssr/orbit/" + gDevice + "/" + satellite + "/v1";
    auto utc_time = ts::Utc{time};

    std::stringstream ss;
    ss << std::setprecision(14);
    ss << "{";
    ss << "\"time\":\"" << utc_time.rfc3339() << "\"";
    if (delta.valid) {
        ss << ",\"delta_radial\":" << delta.value.x;
        ss << ",\"delta_along\":" << delta.value.y;
        ss << ",\"delta_cross\":" << delta.value.z;
    }
    if (dot_delta.valid) {
        ss << ",\"delta_radial_rate\":" << dot_delta.value.x;
        ss << ",\"delta_along_rate\":" << dot_delta.value.y;
        ss << ",\"delta_cross_rate\":" << dot_delta.value.z;
    }
    if (ssr_iod.valid) ss << ",\"ssr_iod\":" << ssr_iod.value;
    if (eph_iod.valid) ss << ",\"eph_iod\":" << eph_iod.value;
    ss << "}";

    publish(topic, ss.str());
}

void report_ssr_clock_correction(ts::Tai const& time, std::string const& satellite,
                                 Option<double> c0, Option<double> c1, Option<double> c2,
                                 Option<long> ssr_iod) {
    FUNCTION_SCOPE();

    auto topic    = "datatrace/ssr/clock/" + gDevice + "/" + satellite + "/v1";
    auto utc_time = ts::Utc{time};

    std::stringstream ss;
    ss << std::setprecision(14);
    ss << "{";
    ss << "\"time\":\"" << utc_time.rfc3339() << "\"";
    if (c0.valid) ss << ",\"c0\":" << c0.value;
    if (c1.valid) ss << ",\"c1\":" << c1.value;
    if (c2.valid) ss << ",\"c2\":" << c2.value;
    if (ssr_iod.valid) ss << ",\"ssr_iod\":" << ssr_iod.value;
    ss << "}";

    publish(topic, ss.str());
}

void report_ssr_ionospheric_polynomial(ts::Tai const& time, std::string const& satellite,
                                       Option<double> c00, Option<double> c01, Option<double> c10,
                                       Option<double> c11, Option<double> reference_point_latitude,
                                       Option<double> reference_point_longitude,
                                       Option<double> stec_quality_indicator,
                                       Option<long>   stec_quality_indicator_cls,
                                       Option<long>   stec_quality_indicator_val,
                                       Option<long>   ssr_iod) {
    FUNCTION_SCOPE();

    auto topic    = "datatrace/ssr/iono_poly/" + gDevice + "/" + satellite + "/v1";
    auto utc_time = ts::Utc{time};

    std::stringstream ss;
    ss << std::setprecision(14);
    ss << "{";
    ss << "\"time\":\"" << utc_time.rfc3339() << "\"";
    if (c00.valid) ss << ",\"c00\":" << c00.value;
    if (c01.valid) ss << ",\"c01\":" << c01.value;
    if (c10.valid) ss << ",\"c10\":" << c10.value;
    if (c11.valid) ss << ",\"c11\":" << c11.value;
    if (reference_point_latitude.valid)
        ss << ",\"reference_point_latitude\":" << reference_point_latitude.value;
    if (reference_point_longitude.valid)
        ss << ",\"reference_point_longitude\":" << reference_point_longitude.value;
    if (stec_quality_indicator.valid)
        ss << ",\"stec_quality_indicator\":" << stec_quality_indicator.value;
    if (stec_quality_indicator_cls.valid)
        ss << ",\"stec_quality_indicator_cls\":" << stec_quality_indicator_cls.value;
    if (stec_quality_indicator_val.valid)
        ss << ",\"stec_quality_indicator_val\":" << stec_quality_indicator_val.value;
    if (ssr_iod.valid) ss << ",\"ssr_iod\":" << ssr_iod.value;
    ss << "}";

    publish(topic, ss.str());
}

void report_ssr_tropospheric_grid(ts::Tai const& time, long grid_point_id,
                                  Option<Float3> position_llh, Option<double> tropo_wet,
                                  Option<double> tropo_dry, Option<long> ssr_iod) {
    FUNCTION_SCOPE();

    auto topic =
        "datatrace/ssr/tropo_grid/" + gDevice + "/" + std::to_string(grid_point_id) + "/v1";
    auto utc_time = ts::Utc{time};

    std::stringstream ss;
    ss << std::setprecision(14);
    ss << "{";
    ss << "\"time\":\"" << utc_time.rfc3339() << "\"";
    if (position_llh.valid) {
        ss << ",\"pos_lat\":" << position_llh.value.x;
        ss << ",\"pos_lon\":" << position_llh.value.y;
    }
    if (tropo_wet.valid) ss << ",\"tropo_wet\":" << tropo_wet.value;
    if (tropo_dry.valid) ss << ",\"tropo_dry\":" << tropo_dry.value;
    if (ssr_iod.valid) ss << ",\"ssr_iod\":" << ssr_iod.value;
    ss << "}";

    publish(topic, ss.str());
}

void report_ssr_ionospheric_grid(ts::Tai const& time, long grid_point_id,
                                 Option<Float3> position_llh, std::string const& satellite,
                                 Option<double> residual, Option<long> ssr_iod) {
    FUNCTION_SCOPE();

    auto topic = "datatrace/ssr/iono_grid/" + gDevice + "/" + std::to_string(grid_point_id) + "/" +
                 satellite + "/v1";
    auto utc_time = ts::Utc{time};

    std::stringstream ss;
    ss << std::setprecision(14);
    ss << "{";
    ss << "\"time\":\"" << utc_time.rfc3339() << "\"";
    if (position_llh.valid) {
        ss << ",\"pos_lat\":" << position_llh.value.x;
        ss << ",\"pos_lon\":" << position_llh.value.y;
    }
    if (residual.valid) ss << ",\"residual\":" << residual.value;
    if (ssr_iod.valid) ss << ",\"ssr_iod\":" << ssr_iod.value;
    ss << "}";

    publish(topic, ss.str());
}

}  // namespace datatrace
