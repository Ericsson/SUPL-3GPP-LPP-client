#pragma once
#include <memory>
#include <receiver/ublox/types.hpp>
#include <receiver/ublox/ubx_cfg.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace interface {
class Interface;
}

namespace receiver {
namespace ublox {

enum class Port : uint8_t {
    UART1,
    UART2,
    I2C,
    USB,
};

enum class MessageId : uint16_t {
    UbxNavPvt = 0x0107,
};

class Message;
class Parser;
class UbloxReceiver {
public:
    /// Construct a receiver. This will block until the receiver configuration has been
    /// acquired.
    UBLOX_EXPLICIT UbloxReceiver(Port                                  port,
                                 std::unique_ptr<interface::Interface> interface) UBLOX_NOEXCEPT;
    ~UbloxReceiver() UBLOX_NOEXCEPT;

    /// Enable message to be sent by the receiver periodically. This will only take effect
    /// after the next call to configure().
    /// @param message_id The message to enable.
    void enable_message(MessageId message_id);

    /// Disable message to be sent by the receiver periodically. This will only take effect
    /// after the next call to configure().
    /// @param message_id The message to disable.
    void disable_message(MessageId message_id);

    /// Load the receiver configuration.
    void load_configuration();

    /// Store the receiver configuration.
    void store_configuration();

    /// Read bytes from the interface and append them to the parse buffer. This will not
    /// block.
    void process();

    ///  Block until a message is available.
    /// @return A pointer to the message, or nullptr if the interface is closed.
    UBLOX_NODISCARD std::unique_ptr<Message> wait_for_message();

    ///  Try to parse a message in the parse buffer. This will not block.
    /// @return A pointer to the message, or nullptr if no message is available.
    UBLOX_NODISCARD std::unique_ptr<Message> try_parse();

    /// Get the software version of the receiver.
    /// @return The software version.
    UBLOX_NODISCARD const std::string& software_version() const UBLOX_NOEXCEPT {
        return mSoftwareVersion;
    }

    /// Get the hardware version of the receiver.
    /// @return The hardware version.
    UBLOX_NODISCARD const std::string& hardware_version() const UBLOX_NOEXCEPT {
        return mHardwareVersion;
    }

    /// Get the extensions of the receiver.
    /// @return The extensions.
    UBLOX_NODISCARD const std::vector<std::string>& extensions() const UBLOX_NOEXCEPT {
        return mExtensions;
    }

    /// The receiver supports the SPARTN protocol.
    /// @return True if the receiver supports the SPARTN protocol.
    UBLOX_NODISCARD bool spartn_support() const UBLOX_NOEXCEPT { return mSpartnSupport; }

    /// The receiver interface.
    /// @return The receiver interface.
    UBLOX_NODISCARD interface::Interface& interface() const UBLOX_NOEXCEPT { return *mInterface; }

protected:
    void poll_mon_ver();
    bool poll_cfg_valget(CfgKey key, CfgValue& value);

    void load_receiver_configuration();
    void store_receiver_configuration();

    template <typename T>
    std::unique_ptr<T> wait_for_specific_message(bool expect_ack, bool expect_nak);

private:
    Port                                  mPort;
    std::unique_ptr<interface::Interface> mInterface;
    Parser*                               mParser;
    bool                                  mSpartnSupport;
    std::unordered_map<CfgKey, CfgValue>  mConfig;

    std::string              mSoftwareVersion;
    std::string              mHardwareVersion;
    std::vector<std::string> mExtensions;
};

}  // namespace ublox
}  // namespace receiver
