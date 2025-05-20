#pragma once
#include <io/input.hpp>
#include <io/output.hpp>
#include <loglet/loglet.hpp>
#include <lpp/assistance_data.hpp>
#include <supl/cell.hpp>

#ifdef INCLUDE_GENERATOR_SPARTN
#include <generator/spartn2/generator.hpp>
#endif

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct LocationServerConfig {
    std::string                  host;
    uint16_t                     port;
    std::unique_ptr<std::string> interface;

    bool enabled;
    bool slp_host_cell;
    bool slp_host_imsi;
    bool shutdown_on_disconnect;
    bool hack_bad_transaction_initiator;
};

struct IdentityConfig {
    bool wait_for_identity;
    bool use_supl_identity_fix;

    std::unique_ptr<uint64_t>    msisdn;
    std::unique_ptr<uint64_t>    imsi;
    std::unique_ptr<std::string> ipv4;
};

LOGLET_MODULE_FORWARD_REF(output);
#define OUTPUT_PRINT_MODULE &LOGLET_MODULE_REF(output)

using OutputFormat                                   = uint64_t;
constexpr static OutputFormat OUTPUT_FORMAT_NONE     = 0;
constexpr static OutputFormat OUTPUT_FORMAT_UBX      = 1;
constexpr static OutputFormat OUTPUT_FORMAT_NMEA     = 2;
constexpr static OutputFormat OUTPUT_FORMAT_RTCM     = 4;
constexpr static OutputFormat OUTPUT_FORMAT_CTRL     = 8;
constexpr static OutputFormat OUTPUT_FORMAT_LPP_XER  = 16;
constexpr static OutputFormat OUTPUT_FORMAT_LPP_UPER = 32;
constexpr static OutputFormat OUTPUT_FORMAT_UNUSED64 = 64;
constexpr static OutputFormat OUTPUT_FORMAT_SPARTN   = 128;
constexpr static OutputFormat OUTPUT_FORMAT_LFR      = 256;
constexpr static OutputFormat OUTPUT_FORMAT_POSSIB   = 512;
constexpr static OutputFormat OUTPUT_FORMAT_TEST     = 1llu << 63;
constexpr static OutputFormat OUTPUT_FORMAT_ALL =
    OUTPUT_FORMAT_UBX | OUTPUT_FORMAT_NMEA | OUTPUT_FORMAT_RTCM | OUTPUT_FORMAT_CTRL |
    OUTPUT_FORMAT_LPP_XER | OUTPUT_FORMAT_LPP_UPER | OUTPUT_FORMAT_SPARTN | OUTPUT_FORMAT_LFR | OUTPUT_FORMAT_POSSIB;

struct OutputInterface {
    OutputFormat                format;
    std::unique_ptr<io::Output> interface;
    bool                        print;

    inline bool ubx_support() const { return (format & OUTPUT_FORMAT_UBX) != 0; }
    inline bool nmea_support() const { return (format & OUTPUT_FORMAT_NMEA) != 0; }
    inline bool rtcm_support() const { return (format & OUTPUT_FORMAT_RTCM) != 0; }
    inline bool ctrl_support() const { return (format & OUTPUT_FORMAT_CTRL) != 0; }
    inline bool lpp_xer_support() const { return (format & OUTPUT_FORMAT_LPP_XER) != 0; }
    inline bool lpp_uper_support() const { return (format & OUTPUT_FORMAT_LPP_UPER) != 0; }
    inline bool spartn_support() const { return (format & OUTPUT_FORMAT_SPARTN) != 0; }
    inline bool lfr_support() const { return (format & OUTPUT_FORMAT_LFR) != 0; }
    inline bool possib_support() const { return (format & OUTPUT_FORMAT_POSSIB) != 0; }

    inline bool test_support() const { return (format & OUTPUT_FORMAT_TEST) != 0; }
};

struct OutputConfig {
    std::vector<OutputInterface> outputs;
};

using InputFormat                                      = uint64_t;
constexpr static InputFormat INPUT_FORMAT_NONE         = 0;
constexpr static InputFormat INPUT_FORMAT_UBX          = 1;
constexpr static InputFormat INPUT_FORMAT_NMEA         = 2;
constexpr static InputFormat INPUT_FORMAT_RTCM         = 4;
constexpr static InputFormat INPUT_FORMAT_CTRL         = 8;
constexpr static InputFormat INPUT_FORMAT_LPP_UPER     = 16;
constexpr static InputFormat INPUT_FORMAT_LPP_UPER_PAD = 32;
constexpr static InputFormat INPUT_FORMAT_ALL          = INPUT_FORMAT_UBX | INPUT_FORMAT_NMEA |
                                                INPUT_FORMAT_RTCM | INPUT_FORMAT_CTRL |
                                                INPUT_FORMAT_LPP_UPER;

struct InputInterface {
    InputFormat                format;
    bool                       print;
    std::unique_ptr<io::Input> interface;
};

struct InputConfig {
    std::vector<InputInterface> inputs;
};

struct AssistanceDataConfig {
    bool enabled;
    bool wait_for_cell;

    lpp::RequestAssistanceData::Type type;
    supl::Cell                       cell;

    bool gps;
    bool glonass;
    bool galileo;
    bool beidou;

    long rtk_observations;
    long rtk_residuals;
    long rtk_bias_information;
    long rtk_reference_station_info;

    long ssr_clock;
    long ssr_orbit;
    long ssr_code_bias;
    long ssr_phase_bias;
    long ssr_stec;
    long ssr_gridded;
    long ssr_ura;
    long ssr_correction_points;

    long delivery_amount;
    bool antenna_height;
};

struct FakeLocationInformationConfig {
    bool   enabled;
    double latitude;
    double longitude;
    double altitude;
};

struct LocationInformationConfig {
    bool unsolicited;
    bool update_rate_forced;
    int  update_rate_ms;

    bool use_nmea_location;
    bool use_ubx_location;

    bool   convert_confidence_95_to_68;
    bool   output_ellipse_68;
    double override_horizontal_confidence;

    FakeLocationInformationConfig fake;
};

#ifdef DATA_TRACING
struct DataTracingConfig {
    bool        enabled;
    std::string device;
    std::string server;
    uint16_t    port;
    std::string username;
    std::string password;

    bool reliable;
    bool disable_ssr_data;
    bool possib_log;
    bool possib_wrap;
};
#endif

struct LoggingConfig {
    loglet::Level                                  log_level;
    bool                                           color;
    bool                                           flush;
    bool                                           tree;
    std::unordered_map<std::string, loglet::Level> module_levels;
};

struct GnssConfig {
    bool gps;
    bool glonass;
    bool galileo;
    bool beidou;
};

#ifdef INCLUDE_GENERATOR_RTCM
struct Lpp2RtcmConfig {
    enum class MsmType {
        ANY,
        MSM4,
        MSM5,
        MSM6,
        MSM7,
    };

    bool    enabled;
    bool    generate_gps;
    bool    generate_glonass;
    bool    generate_galileo;
    bool    generate_beidou;
    MsmType msm_type;
};

struct Lpp2FrameRtcmConfig {
    bool enabled;
    int  rtcm_message_id;
    bool output_in_rtcm;
};
#endif

#ifdef INCLUDE_GENERATOR_SPARTN
struct Lpp2SpartnConfig {
    bool enabled;
    bool generate_gps;
    bool generate_glonass;
    bool generate_galileo;
    bool generate_beidou;

    int sf024_override;
    int sf024_default;
    int sf042_override;
    int sf042_default;
    int sf055_override;
    int sf055_default;

    bool ublox_clock_correction;
    bool average_zenith_delay;

    bool                          increasing_siou;
    bool                          filter_by_residuals;
    bool                          filter_by_ocb;
    bool                          ignore_l2l;
    bool                          hydrostatic_in_zenith;
    generator::spartn::StecMethod stec_method;
    bool                          stec_transform;
    bool                          stec_invalid_to_zero;
    bool                          sign_flip_c00;
    bool                          sign_flip_c01;
    bool                          sign_flip_c10;
    bool                          sign_flip_c11;
    bool                          sign_flip_stec_residuals;
    bool                          code_bias_translate;
    bool                          code_bias_correction_shift;
    bool                          phase_bias_translate;
    bool                          phase_bias_correction_shift;
    bool                          generate_gad;
    bool                          generate_ocb;
    bool                          generate_hpac;
    bool                          flip_grid_bitmask;
    bool                          flip_orbit_correction;
};
#endif

#ifdef INCLUDE_GENERATOR_TOKORO
struct TokoroConfig {
    enum class VrsMode {
        Fixed,
        Dynamic,
    };

    enum class GenerationStrategy {
        AssistanceData,   // Generate when assistance data is received
        TimeStep,         // Generate every time step, using the current time
        TimeStepAligned,  // Generate every time step, using the current time floored to the nearest
                          // time step
        TimeStepLast,     // Generate every time step, using the same time step as the last
                          // assistance data
    };

    bool enabled;
    bool generate_gps;
    bool generate_glonass;
    bool generate_galileo;
    bool generate_beidou;

    VrsMode            vrs_mode;
    GenerationStrategy generation_strategy;
    double dynamic_distance_threshold;  // in km, <= 0.0 means no threshold (update every time)

    double fixed_itrf_x;
    double fixed_itrf_y;
    double fixed_itrf_z;
    double fixed_rtcm_x;
    double fixed_rtcm_y;
    double fixed_rtcm_z;

    double time_step;

    bool shapiro_correction;
    bool phase_windup_correction;
    bool earth_solid_tides_correction;
    bool antenna_phase_variation_correction;
    bool tropospheric_height_correction;
    bool iod_consistency_check;
    bool rtoc;
    bool ocit;
    bool negative_phase_windup;
    bool generate_rinex;

    bool require_code_bias;
    bool require_phase_bias;
    bool require_tropo;
    bool require_iono;

    bool use_tropospheric_model;
    bool use_ionospheric_height_correction;

    std::string antex_file;
};
#endif

struct Config {
    LocationServerConfig      location_server;
    IdentityConfig            identity;
    AssistanceDataConfig      assistance_data;
    LocationInformationConfig location_information;
    OutputConfig              output;
    InputConfig               input;
    GnssConfig                gnss;
#ifdef INCLUDE_GENERATOR_RTCM
    Lpp2RtcmConfig      lpp2rtcm;
    Lpp2FrameRtcmConfig lpp2frame_rtcm;
#endif
#ifdef INCLUDE_GENERATOR_SPARTN
    Lpp2SpartnConfig lpp2spartn;
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
    TokoroConfig tokoro;
#endif
    LoggingConfig logging;
#ifdef DATA_TRACING
    DataTracingConfig data_tracing;
#endif
};

namespace config {
bool parse(int argc, char** argv, Config* config);
void dump(Config* config);
}  // namespace config
