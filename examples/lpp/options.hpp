#pragma once
#include <args.hpp>
#include <interface/interface.hpp>
#include <memory>
#include <receiver/ublox/receiver.hpp>
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
    /// Identify the device with MSISDN.
    std::unique_ptr<unsigned long> msisdn;
    /// Identify the device with IMSI.
    std::unique_ptr<unsigned long> imsi;
    /// Identify the device with IPv4 address.
    std::unique_ptr<std::string> ipv4;
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
    int cid;
};

struct ModemDevice {
    std::string device;
    int         baud_rate;
};

/// Modem options.
struct ModemOptions {
    std::unique_ptr<ModemDevice> device;
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
};

/// Options.
struct Options {
    LocationServerOptions location_server_options;
    IdentityOptions       identity_options;
    CellOptions           cell_options;
    ModemOptions          modem_options;
    OutputOptions         output_options;
    UbloxOptions          ublox_options;
};

/// Command.
class Command {
public:
    Command(std::string name, std::string description)
        : mName(std::move(name)), mDescription(std::move(description)) {}
    virtual ~Command() = default;

    /// Name of the command. This is used to invoke the command.
    const std::string& name() const { return mName; }
    /// Description of the command.
    const std::string& description() const { return mDescription; }

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
