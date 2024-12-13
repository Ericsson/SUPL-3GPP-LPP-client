#pragma once
#include <io/input.hpp>
#include <io/output.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-dtor"
#pragma GCC diagnostic ignored "-Wnewline-eof"
#pragma GCC diagnostic ignored "-Wmissing-variable-declarations"
#pragma GCC diagnostic ignored "-Winconsistent-missing-destructor-override"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wshadow-field"
#include <args.hpp>
#pragma GCC diagnostic pop

#include <memory>
#include <string>
#include <vector>

#include <loglet/loglet.hpp>

/// Location server options.
struct LocationServerOptions {
    /// Hostname or IP address of the location server.
    std::string host;
    /// Port of the location server. Which is usually 5431 for non-SSL.
    int port;
    /// Whether to use SSL or not.
    bool ssl;

    bool skip_connect;
    bool skip_request_assistance_data;
};

/// Identity options.
struct IdentityOptions {
    /// Wait for the identity to be provided via the control interface.
    bool wait_for_identity;
    /// Identify the device with MSISDN.
    std::unique_ptr<unsigned long long> msisdn;
    /// Identify the device with IMSI.
    std::unique_ptr<unsigned long long> imsi;
    /// Identify the device with IPv4 address.
    std::unique_ptr<std::string> ipv4;

    /// Whether to switch the order of the digits in the identity.
    bool use_supl_identity_fix;
};

/// Cell options.
struct CellOptions {
    /// Mobile Country Code.
    int mcc;
    /// Mobile Network Code.
    int mnc;
    /// Tracking Area Code.
    int tac;
    /// Cell ID.
    unsigned long long cid;
    /// Is NR cell.
    bool is_nr;
};

using OutputFormat                                         = uint64_t;
constexpr static OutputFormat OUTPUT_FORMAT_NONE           = 0;
constexpr static OutputFormat OUTPUT_FORMAT_UBX            = 1;
constexpr static OutputFormat OUTPUT_FORMAT_NMEA           = 2;
constexpr static OutputFormat OUTPUT_FORMAT_RTCM           = 4;
constexpr static OutputFormat OUTPUT_FORMAT_CTRL           = 8;
constexpr static OutputFormat OUTPUT_FORMAT_LPP_XER        = 16;
constexpr static OutputFormat OUTPUT_FORMAT_LPP_UPER       = 32;
constexpr static OutputFormat OUTPUT_FORMAT_LPP_RTCM_FRAME = 64;
constexpr static OutputFormat OUTPUT_FORMAT_SPARTN         = 128;
constexpr static OutputFormat OUTPUT_FORMAT_ALL =
    OUTPUT_FORMAT_UBX | OUTPUT_FORMAT_NMEA | OUTPUT_FORMAT_RTCM | OUTPUT_FORMAT_CTRL |
    OUTPUT_FORMAT_LPP_XER | OUTPUT_FORMAT_LPP_UPER | OUTPUT_FORMAT_LPP_RTCM_FRAME |
    OUTPUT_FORMAT_SPARTN;

struct OutputOption {
    OutputFormat                format;
    std::unique_ptr<io::Output> interface;
};

/// Output options.
struct OutputOptions {
    std::vector<OutputOption> outputs;
};

using InputFormat                              = uint64_t;
constexpr static InputFormat INPUT_FORMAT_NONE = 0;
constexpr static InputFormat INPUT_FORMAT_UBX  = 1;
constexpr static InputFormat INPUT_FORMAT_NMEA = 2;
constexpr static InputFormat INPUT_FORMAT_RTCM = 4;
constexpr static InputFormat INPUT_FORMAT_CTRL = 8;
constexpr static InputFormat INPUT_FORMAT_LPP  = 16;
constexpr static InputFormat INPUT_FORMAT_ALL =
    INPUT_FORMAT_UBX | INPUT_FORMAT_NMEA | INPUT_FORMAT_RTCM | INPUT_FORMAT_CTRL | INPUT_FORMAT_LPP;

struct InputOption {
    InputFormat                format;
    std::unique_ptr<io::Input> interface;
};

/// Input options.
struct InputOptions {
    std::vector<InputOption> inputs;
    /// Print UBX messages.
    bool print_ubx;
    /// Print NMEA messages.
    bool print_nmea;
    bool print_ctrl;
};

/// Location information options.
struct LocationInformationOptions {
    /// Enable fake location information.
    bool fake_location_info;
    /// Force location information to be sent, even if it hasn't been requested.
    bool force;
    /// Fake latitude.
    double latitude;
    /// Fake longitude.
    double longitude;
    /// Fake altitude.
    double altitude;
    /// Unlock update rate.
    bool unlock_update_rate;
    /// Convert confidence 95% to 39%.
    bool convert_confidence_95_to_39;
    /// Convert confidence 95% to 68%.
    bool convert_confidence_95_to_68;
    /// Output error ellipse with confidence 68% instead of 39%.
    bool output_ellipse_68;
    /// Override horizontal confidence.
    double override_horizontal_confidence;
    /// Don't use location information from NMEA messages.
    bool disable_nmea_location;
    /// Don't use location information from UBX messages.
    bool disable_ubx_location;
    /// Update rate in milliseconds.
    int update_rate;
};

struct DataTracingOptions {
    std::string device;
    std::string server;
    int         port;
    std::string username;
    std::string password;
};

/// Options.
struct Options {
    loglet::Level                                  log_level;
    std::unordered_map<std::string, loglet::Level> module_levels;

    std::unique_ptr<DataTracingOptions> data_tracing;

    LocationServerOptions      location_server_options;
    IdentityOptions            identity_options;
    CellOptions                cell_options;
    InputOptions               input_options;
    OutputOptions              output_options;
    LocationInformationOptions location_information_options;
};

/// Command.
class Command {
public:
    Command(std::string name, std::string description)
        : mName(std::move(name)), mDescription(std::move(description)) {}
    virtual ~Command() = default;

    /// Name of the command. This is used to invoke the command.
    std::string const& name() const { return mName; }
    /// Description of the command.
    std::string const& description() const { return mDescription; }

    /// Parse additional arguments for the command.
    virtual void parse(args::Subparser& parser) = 0;
    /// Execute the command.
    virtual void execute(Options options) = 0;

protected:
    std::string                    mName;
    std::string                    mDescription;
    std::unique_ptr<args::Command> mCommand;
};

/// Option parser.
class OptionParser {
public:
    OptionParser();

    /// Add a command.
    void add_command(Command* command);

    /// Add a command.
    void add_command(std::unique_ptr<Command> command);

    /// Parse and execute the command.
    int parse_and_execute(int argc, char** argv);

private:
    std::vector<std::unique_ptr<Command>> mCommands;
};
