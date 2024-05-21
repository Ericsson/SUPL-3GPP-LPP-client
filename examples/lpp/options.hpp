#pragma once
#include <interface/interface.hpp>
#include <receiver/ublox/receiver.hpp>

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

/// Location server options.
struct LocationServerOptions {
    /// Hostname or IP address of the location server.
    std::string host;
    /// Port of the location server. Which is usually 5431 for non-SSL.
    int port;
    /// Whether to use SSL or not.
    bool ssl;
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

/// Output options.
struct OutputOptions {
    /// Interfaces to output data to.
    std::vector<std::unique_ptr<interface::Interface>> interfaces;
};

/// Ublox options.
struct UbloxOptions {
    /// Port on the u-blox receiver.
    receiver::ublox::Port port;
    /// Interface to use for communication with the receiver.
    std::unique_ptr<interface::Interface> interface;
    /// Whether to print messages.
    bool print_messages;
};

/// Nmea options.
struct NmeaOptions {
    /// Interface to use for communication with the receiver.
    std::unique_ptr<interface::Interface> interface;
    /// Whether to print messages.
    bool print_messages;
    /// Export messages to other interfaces.
    std::vector<std::unique_ptr<interface::Interface>> export_interfaces;
};

/// Location information options.
struct LocationInformationOptions {
    /// Enable fake location information.
    bool enabled;
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
    /// Override horizontal confidence.
    double override_horizontal_confidence;
};

/// Control options.
struct ControlOptions {
    /// Control interface.
    std::unique_ptr<interface::Interface> interface;
};

/// Options.
struct Options {
    LocationServerOptions      location_server_options;
    IdentityOptions            identity_options;
    CellOptions                cell_options;
    OutputOptions              output_options;
    UbloxOptions               ublox_options;
    NmeaOptions                nmea_options;
    LocationInformationOptions location_information_options;
    ControlOptions             control_options;
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
