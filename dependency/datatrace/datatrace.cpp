#include "datatrace.hpp"

#include <iomanip>
#include <mosquitto.h>
#include <sstream>

#include <loglet/loglet.hpp>
#include <time/utc.hpp>
#define LOGLET_CURRENT_MODULE "datatrace"

namespace datatrace {

static void on_connect(struct mosquitto*, void*, int reason_code) {
    VERBOSEF("on_connect: %s", mosquitto_connack_string(reason_code));
    if (reason_code != 0) {
#if LIBMOSQUITTO_MAJOR >= 2
        ERRORF("connection failed: %s", mosquitto_reason_string(reason_code));
#else
        ERRORF("connection failed");
#endif
    }
}

static mosquitto*  gMosq = nullptr;
static std::string gDevice;

void initialize(std::string const& device, std::string const& server, int port,
                std::string const& username, std::string const& password) {
    VSCOPE_FUNCTION();
    ::mosquitto_lib_init();
    VERBOSEF("::mosquitto_lib_init()");

    gDevice = device;

    ASSERT(gMosq == nullptr, "mosq is not null");
    gMosq = ::mosquitto_new(nullptr, true, nullptr);
    VERBOSEF("::mosquitto_new(nullptr, true, nullptr) = %p", gMosq);
    if (gMosq == nullptr) {
        return;
    }

    ::mosquitto_connect_callback_set(gMosq, on_connect);
    VERBOSEF("::mosquitto_connect_callback_set(%p, %p)", gMosq, on_connect);

    auto result = ::mosquitto_username_pw_set(gMosq, username.c_str(), password.c_str());
    VERBOSEF("::mosquitto_username_pw_set(%p, %s, %s) = %d", gMosq, username.c_str(),
             password.c_str(), result);
    if (result != MOSQ_ERR_SUCCESS) {
        ERRORF("failed to set username and password: %s", mosquitto_strerror(result));
        mosquitto_destroy(gMosq);
        return;
    }

    result = ::mosquitto_connect(gMosq, server.c_str(), port, 60);
    VERBOSEF("::mosquitto_connect(%p, %s, %d, 60) = %d", gMosq, server.c_str(), port, result);
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
}

void finalize() {
    VSCOPE_FUNCTION();
    ASSERT(gMosq != nullptr, "mosq is null");

    ::mosquitto_lib_cleanup();
    VERBOSEF("::mosquitto_lib_cleanup()");
}

void replace_all(std::string& str, std::string const& from, std::string const& to) {
    if (from.empty()) return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

void report_observation(ts::Tai const& time, std::string const& satellite,
                        std::string const& signal, Observation const& obs) {
    if (gMosq == nullptr) return;
    VSCOPE_FUNCTION();

    auto signal_copy = signal;
    replace_all(signal_copy, "+", "");
    replace_all(signal_copy, "/", "");
    auto topic    = "datatrace/obs/" + gDevice + "/" + satellite + "/" + signal_copy + "/v1";
    auto utc_time = ts::Utc{time};

    std::stringstream ss;
    ss << std::setprecision(14);
    ss << "{";
    ss << "\"time\":\"" << utc_time.rfc3339() << "\"";
    if (std::abs(obs.frequency) > 1e-6) ss << ",\"freq\":" << obs.frequency;
    if (std::abs(obs.geo_range) > 1e-6) ss << ",\"geo_range\":" << obs.geo_range;
    if (std::abs(obs.sat_clock) > 1e-6) ss << ",\"sat_clock\":" << obs.sat_clock;
    if (std::abs(obs.clock) > 1e-6) ss << ",\"clock\":" << obs.clock;
    if (std::abs(obs.orbit) > 1e-6) ss << ",\"orbit\":" << obs.orbit;
    if (std::abs(obs.code_bias) > 1e-6) ss << ",\"code_bias\":" << obs.code_bias;
    if (std::abs(obs.phase_bias) > 1e-6) ss << ",\"phase_bias\":" << obs.phase_bias;
    if (std::abs(obs.stec_grid) > 1e-6) ss << ",\"stec_grid\":" << obs.stec_grid;
    if (std::abs(obs.stec_poly) > 1e-6) ss << ",\"stec_poly\":" << obs.stec_poly;
    if (std::abs(obs.tropo_dry) > 1e-6) ss << ",\"tropo_dry\":" << obs.tropo_dry;
    if (std::abs(obs.tropo_wet) > 1e-6) ss << ",\"tropo_wet\":" << obs.tropo_wet;
    if (std::abs(obs.shapiro) > 1e-6) ss << ",\"shapiro\":" << obs.shapiro;
    if (std::abs(obs.earth_solid_tides) > 1e-6) ss << ",\"est\":" << obs.earth_solid_tides;
    if (std::abs(obs.phase_windup) > 1e-6) ss << ",\"pw\":" << obs.phase_windup;
    if (std::abs(obs.antenna_phase_variation) > 1e-6)
        ss << ",\"apv\":" << obs.antenna_phase_variation;
    if (std::abs(obs.code_range) > 1e-6) ss << ",\"code_range\":" << obs.code_range;
    if (std::abs(obs.phase_range) > 1e-6) ss << ",\"phase_range\":" << obs.phase_range;
    if (std::abs(obs.phase_range_rate) > 1e-6)
        ss << ",\"phase_range_rate\":" << obs.phase_range_rate;
    if (std::abs(obs.carrier_to_noise_ratio) > 1e-6)
        ss << ",\"carrier_to_noise_ratio\":" << obs.carrier_to_noise_ratio;
    if (std::abs(obs.phase_lock_time) > 1e-6) ss << ",\"phase_lock_time\":" << obs.phase_lock_time;
    ss << "}";

    auto ss_data = ss.str();
    auto result  = ::mosquitto_publish(gMosq, nullptr, topic.c_str(), ss_data.size(),
                                       ss_data.c_str(), 0, false);
    VERBOSEF("::mosquitto_publish(%p, nullptr, \"%s\", %zu, ..., 0, false) = %d", gMosq,
             topic.c_str(), ss_data.size(), result);
    if (result != MOSQ_ERR_SUCCESS) {
        WARNF("failed to publish observation: %s", mosquitto_strerror(result));
        return;
    }
}

void report_satellite(ts::Tai const& time, std::string const& satellite, Satellite const& sat) {
    if (gMosq == nullptr) return;
    VSCOPE_FUNCTION();

    auto topic    = "datatrace/sat/" + gDevice + "/" + satellite + "/v1";
    auto utc_time = ts::Utc{time};

    std::stringstream ss;
    ss << std::setprecision(14);
    ss << "{";
    ss << "\"time\":\"" << utc_time.rfc3339() << "\"";
    ss << ",\"pos_x\":" << sat.position.x;
    ss << ",\"pos_y\":" << sat.position.y;
    ss << ",\"pos_z\":" << sat.position.z;
    ss << ",\"vel_x\":" << sat.velocity.x;
    ss << ",\"vel_y\":" << sat.velocity.y;
    ss << ",\"vel_z\":" << sat.velocity.z;
    ss << ",\"elevation\":" << sat.elevation;
    ss << ",\"azimuth\":" << sat.azimuth;
    ss << ",\"iod\":" << sat.iod;
    ss << "}";

    auto ss_data = ss.str();
    auto result  = ::mosquitto_publish(gMosq, nullptr, topic.c_str(), ss_data.size(),
                                       ss_data.c_str(), 0, false);
    VERBOSEF("::mosquitto_publish(%p, nullptr, \"%s\", %zu, ..., 0, false) = %d", gMosq,
             topic.c_str(), ss_data.size(), result);
    if (result != MOSQ_ERR_SUCCESS) {
        WARNF("failed to publish satellite: %s", mosquitto_strerror(result));
        return;
    }
}

}  // namespace datatrace
