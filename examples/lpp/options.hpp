#pragma once
#include <args.hpp>
#include <interface/interface.hpp>
#include <memory>
#include <string>
#include <vector>

/// @brief Location server options.
struct LocationServerOptions {
    /// @brief Hostname or IP address of the location server.
    std::string host;
    /// @brief Port of the location server. Which is usually 5431 for non-SSL.
    int port;
    /// @brief Whether to use SSL or not.
    bool ssl;
};

/// @brief Identity options.
struct IdentityOptions {
    /// @brief Identify the device with MSISDN.
    std::unique_ptr<unsigned long> msisdn;
    /// @brief Identify the device with IMSI.
    std::unique_ptr<unsigned long> imsi;
    /// @brief Identify the device with IPv4 address.
    std::unique_ptr<std::string> ipv4;
};

/// @brief Cell options.
struct CellOptions {
    /// @brief Mobile Country Code.
    int mcc;
    /// @brief Mobile Network Code.
    int mnc;
    /// @brief Tracking Area Code.
    int tac;
    /// @brief Cell ID.
    int cid;
};

struct ModemDevice {
    std::string device;
    int         baud_rate;
};

/// @brief Modem options.
struct ModemOptions {
    std::unique_ptr<ModemDevice> device;
};

/// @brief Output options.
struct OutputOptions {
    /// @brief Interfaces to output data to.
    std::vector<std::unique_ptr<interface::Interface>> interfaces;
};

/// @brief Options.
struct Options {
    LocationServerOptions location_server_options;
    IdentityOptions       identity_options;
    CellOptions           cell_options;
    ModemOptions          modem_options;
    OutputOptions         output_options;
};

/// @brief Command.
class Command {
public:
    Command(std::string name, std::string description)
        : mName(std::move(name)), mDescription(std::move(description)) {}
    virtual ~Command() = default;

    /// @brief Name of the command. This is used to invoke the command.
    const std::string& name() const { return mName; }
    /// @brief Description of the command.
    const std::string& description() const { return mDescription; }

    /// @brief Parse additional arguments for the command.
    virtual void parse(args::Subparser& parser) = 0;
    /// @brief Execute the command.
    virtual void execute(Options options) = 0;

protected:
    std::string                    mName;
    std::string                    mDescription;
    std::unique_ptr<args::Command> mCommand;
};

/// @brief Option parser.
class OptionParser {
public:
    OptionParser();

    /// @brief Add a command.
    void add_command(Command* command);

    /// @brief Add a command.
    void add_command(std::unique_ptr<Command> command);

    /// @brief Parse and execute the command.
    int parse_and_execute(int argc, char** argv);

private:
    std::vector<std::unique_ptr<Command>> mCommands;
};
