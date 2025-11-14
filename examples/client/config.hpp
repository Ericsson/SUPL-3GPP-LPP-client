#pragma once
#include <io/input.hpp>
#include <io/output.hpp>
#include <loglet/loglet.hpp>
#include <lpp/assistance_data.hpp>
#include <supl/cell.hpp>

#ifdef INCLUDE_GENERATOR_SPARTN
#include <generator/spartn2/generator.hpp>
#endif

#ifdef INCLUDE_GENERATOR_IDOKEIDO
#include <generator/idokeido/idokeido.hpp>
#endif

#include "input_format.hpp"
#include "output_format.hpp"
#include "stage.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <format/ubx/cfg.hpp>

struct LocationServerConfig {
    std::string                  host;
    uint16_t                     port;
    std::unique_ptr<std::string> interface;

    bool enabled;
    bool slp_host_cell;
    bool slp_host_imsi;
    bool shutdown_on_disconnect;
    bool hack_bad_transaction_initiator;
    bool hack_never_send_abort;
};

enum class AGnssMode {
    Periodic,
    Triggered,
    Both,
};

struct AGnssConfig {
    std::string                  host;
    uint16_t                     port;
    std::unique_ptr<std::string> interface;

    bool      enabled;
    AGnssMode mode;
    bool      gps;
    bool      glonass;
    bool      galileo;
    bool      beidou;
    long      interval_seconds;
    long      triggered_cooldown_seconds;

    std::unique_ptr<uint64_t>    msisdn;
    std::unique_ptr<uint64_t>    imsi;
    std::unique_ptr<std::string> ipv4;
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

struct OutputInterface {
    OutputFormat                 format;
    std::unique_ptr<io::Output>  initial_interface;
    std::unique_ptr<OutputStage> stage;
    std::vector<std::string>     include_tags;
    std::vector<std::string>     exclude_tags;
    std::vector<std::string>     stages;

    uint64_t include_tag_mask;
    uint64_t exclude_tag_mask;

    static OutputInterface create(OutputFormat format, std::unique_ptr<io::Output> interface,
                                  std::vector<std::string> include_tags,
                                  std::vector<std::string> exclude_tags,
                                  std::vector<std::string> stages) {
        return {
            format,
            std::move(interface),
            nullptr,
            std::move(include_tags),
            std::move(exclude_tags),
            std::move(stages),
            0,
            0,
        };
    }

    NODISCARD inline bool ubx_support() const { return (format & OUTPUT_FORMAT_UBX) != 0; }
    NODISCARD inline bool nmea_support() const { return (format & OUTPUT_FORMAT_NMEA) != 0; }
    NODISCARD inline bool rtcm_support() const { return (format & OUTPUT_FORMAT_RTCM) != 0; }
    NODISCARD inline bool ctrl_support() const { return (format & OUTPUT_FORMAT_CTRL) != 0; }
    NODISCARD inline bool lpp_xer_support() const { return (format & OUTPUT_FORMAT_LPP_XER) != 0; }
    NODISCARD inline bool lpp_uper_support() const {
        return (format & OUTPUT_FORMAT_LPP_UPER) != 0;
    }
    NODISCARD inline bool spartn_support() const { return (format & OUTPUT_FORMAT_SPARTN) != 0; }
    NODISCARD inline bool lfr_support() const { return (format & OUTPUT_FORMAT_LFR) != 0; }
    NODISCARD inline bool possib_support() const { return (format & OUTPUT_FORMAT_POSSIB) != 0; }
    NODISCARD inline bool location_support() const {
        return (format & OUTPUT_FORMAT_LOCATION) != 0;
    }

    NODISCARD inline bool test_support() const { return (format & OUTPUT_FORMAT_TEST) != 0; }

    NODISCARD inline bool accept_tag(uint64_t tag) const {
        return tag == 0 ||
               (((include_tag_mask & tag) || include_tag_mask == 0) && !(exclude_tag_mask & tag));
    }
};

struct OutputConfig {
    std::vector<OutputInterface> outputs;
};

struct PrintInterface {
    OutputFormat             format;
    std::vector<std::string> include_tags;
    std::vector<std::string> exclude_tags;

    uint64_t include_tag_mask;
    uint64_t exclude_tag_mask;

    static PrintInterface create(OutputFormat format, std::vector<std::string> include_tags,
                                 std::vector<std::string> exclude_tags) {
        return {
            format, std::move(include_tags), std::move(exclude_tags), 0, 0,
        };
    }

    NODISCARD inline bool ubx_support() const { return (format & OUTPUT_FORMAT_UBX) != 0; }
    NODISCARD inline bool nmea_support() const { return (format & OUTPUT_FORMAT_NMEA) != 0; }
    NODISCARD inline bool rtcm_support() const { return (format & OUTPUT_FORMAT_RTCM) != 0; }
    NODISCARD inline bool ctrl_support() const { return (format & OUTPUT_FORMAT_CTRL) != 0; }
    NODISCARD inline bool agnss_support() const { return (format & OUTPUT_FORMAT_AGNSS) != 0; }
    NODISCARD inline bool lpp_support() const {
        return (format & OUTPUT_FORMAT_LPP_XER) != 0 || (format & OUTPUT_FORMAT_LPP_UPER) != 0;
    }

    NODISCARD inline bool accept_tag(uint64_t tag) const {
        return tag == 0 ||
               (((include_tag_mask & tag) || include_tag_mask == 0) && !(exclude_tag_mask & tag));
    }
};

struct PrintConfig {
    std::vector<PrintInterface> prints;
};

struct InputInterface {
    InputFormat                format;
    bool                       print;
    std::unique_ptr<io::Input> interface;
    std::vector<std::string>   tags;
    std::vector<std::string>   stages;
    bool                       nmea_lf_only;
    bool                       discard_errors;
    bool                       discard_unknowns;
    bool                       exclude_from_shutdown;
};

struct InputConfig {
    std::vector<InputInterface> inputs;
    bool                        disable_pipe_buffer_optimization;
    bool                        shutdown_on_complete;
    std::chrono::milliseconds   shutdown_delay;
};

struct AssistanceDataConfig {
    bool enabled;
    bool wait_for_cell;
    bool use_latest_cell_on_reconnect;
    bool request_assisted_gnss;

    lpp::PeriodicRequestAssistanceData::Type type;
    supl::Cell                               cell;

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
    bool enable;
    bool unsolicited;
    bool update_rate_forced;
    int  update_rate_ms;

    bool use_nmea_location;
    bool use_ubx_location;

    bool   convert_confidence_95_to_68;
    bool   output_ellipse_68;
    double override_horizontal_confidence;

    bool nmea_require_gst;
    bool nmea_require_vtg;

    std::vector<std::string> nmea_order;
    bool                     nmea_order_strict;

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

enum class UbxPrintMode { NONE, OPTIONS, ALL };

struct UbxConfigInterface {
    std::unique_ptr<io::Output>                                        output_interface;
    std::unique_ptr<io::Input>                                         input_interface;
    std::vector<std::pair<format::ubx::CfgKey, format::ubx::CfgValue>> options;
    UbxPrintMode print_mode = UbxPrintMode::NONE;
};

struct UbxConfigConfig {
    std::vector<UbxConfigInterface> interfaces;
    bool                            apply_and_exit = false;
};

struct LoggingConfig {
    loglet::Level                                  log_level;
    bool                                           color;
    bool                                           flush;
    bool                                           tree;
    bool                                           report_errors;
    bool                                           use_stderr;
    std::unique_ptr<std::string>                   log_file;
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

struct Lpp2EphConfig {
    bool enabled;
    bool gps;
    bool galileo;
    bool beidou;
};

struct Ubx2EphConfig {
    bool enabled;
    bool gps;
    bool galileo;
    bool beidou;
};

struct Rtcm2EphConfig {
    bool enabled;
    bool gps;
    bool galileo;
    bool beidou;
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
    bool                          do_not_use_satellite;
};
#endif

#ifdef INCLUDE_GENERATOR_TOKORO
struct TokoroConfig {
    enum class VrsMode {
        Fixed,
        Dynamic,
        Grid,
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

    std::unique_ptr<std::pair<int, int>> vrs_grid_position;  // east, north

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
    bool        ignore_bitmask;
};
#endif

#ifdef INCLUDE_GENERATOR_IDOKEIDO
struct IdokeidoConfig {
    bool enabled;

    bool gps;
    bool glonass;
    bool galileo;
    bool beidou;

    double      update_rate;
    std::string ephemeris_cache;

    idokeido::WeightFunction    weight_function;
    idokeido::EpochSelection    epoch_selection;
    idokeido::RelativisticModel relativistic_model;
    idokeido::IonosphericMode   ionospheric_mode;
    double                      observation_window;
};
#endif

struct Config {
    LocationServerConfig      location_server;
    AGnssConfig               agnss;
    IdentityConfig            identity;
    AssistanceDataConfig      assistance_data;
    LocationInformationConfig location_information;
    OutputConfig              output;
    InputConfig               input;
    PrintConfig               print;
    GnssConfig                gnss;
#ifdef INCLUDE_GENERATOR_RTCM
    Lpp2RtcmConfig      lpp2rtcm;
    Lpp2FrameRtcmConfig lpp2frame_rtcm;
    Lpp2EphConfig       lpp2eph;
    Ubx2EphConfig       ubx2eph;
    Rtcm2EphConfig      rtcm2eph;
#endif
#ifdef INCLUDE_GENERATOR_SPARTN
    Lpp2SpartnConfig lpp2spartn;
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
    TokoroConfig tokoro;
#endif
#ifdef INCLUDE_GENERATOR_IDOKEIDO
    IdokeidoConfig idokeido;
#endif
    LoggingConfig logging;
#ifdef DATA_TRACING
    DataTracingConfig data_tracing;
#endif
    UbxConfigConfig ubx_config;

    uint64_t                                  next_tag_bit_mask;
    std::unordered_map<std::string, uint64_t> tag_to_bit_mask;

    void register_tag(std::string const& tag) {
        if (tag_to_bit_mask.find(tag) == tag_to_bit_mask.end()) {
            tag_to_bit_mask[tag] = next_tag_bit_mask;
            next_tag_bit_mask    = next_tag_bit_mask << 1;
        }
    }

    uint64_t get_tag(std::string const& tag) {
        if (tag_to_bit_mask.find(tag) == tag_to_bit_mask.end()) {
            return 0;
        }
        return tag_to_bit_mask[tag];
    }

    uint64_t get_tag(char const* tag) { return get_tag(std::string(tag)); }

    uint64_t get_tag(std::vector<std::string> const& tags) {
        uint64_t tag_bit_mask = 0;
        for (auto const& tag : tags) {
            tag_bit_mask |= get_tag(tag);
        }
        return tag_bit_mask;
    }
};

LOGLET_MODULE_FORWARD_REF2(client, config);

namespace args {
class ArgumentParser;
}

namespace ad {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(AssistanceDataConfig const& config);
}  // namespace ad

namespace agnss {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(AGnssConfig const& config);
}  // namespace agnss

namespace gnss {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(GnssConfig const& config);
}  // namespace gnss

namespace identity {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(IdentityConfig const& config);
}  // namespace identity

namespace input {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(InputConfig const& config);
}  // namespace input

namespace li {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(LocationInformationConfig const& config);
}  // namespace li

namespace ls {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(LocationServerConfig const& config);
}  // namespace ls

namespace logging {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(LoggingConfig const& config);
}  // namespace logging

namespace output {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(OutputConfig const& config);
}  // namespace output

namespace print {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(PrintConfig const& config);
}  // namespace print

#ifdef INCLUDE_GENERATOR_RTCM
namespace lpp2rtcm {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(Lpp2RtcmConfig const& config);
}  // namespace lpp2rtcm

namespace lpp2frame_rtcm {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(Lpp2FrameRtcmConfig const& config);
}  // namespace lpp2frame_rtcm

namespace lpp2eph {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(Lpp2EphConfig const& config);
}  // namespace lpp2eph

namespace ubx2eph {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(Ubx2EphConfig const& config);
}  // namespace ubx2eph

namespace rtcm2eph {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(Rtcm2EphConfig const& config);
}  // namespace rtcm2eph
#endif

#ifdef INCLUDE_GENERATOR_SPARTN
namespace lpp2spartn {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(Lpp2SpartnConfig const& config);
}  // namespace lpp2spartn
#endif

#ifdef INCLUDE_GENERATOR_TOKORO
namespace tokoro {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(TokoroConfig const& config);
}  // namespace tokoro
#endif

#ifdef INCLUDE_GENERATOR_IDOKEIDO
namespace idokeido {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(IdokeidoConfig const& config);
}  // namespace idokeido
#endif

#ifdef DATA_TRACING
namespace data_tracing {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(DataTracingConfig const& config);
}  // namespace data_tracing
#endif

namespace ubx_config {
void setup(args::ArgumentParser& parser);
void parse(Config* config);
void dump(UbxConfigConfig const& config);
}  // namespace ubx_config

namespace config {
bool parse(int argc, char** argv, Config* config);
void dump(Config* config);
}  // namespace config
