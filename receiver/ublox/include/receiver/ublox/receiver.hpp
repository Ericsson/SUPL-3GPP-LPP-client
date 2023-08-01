#pragma once
#include <receiver/ublox/types.hpp>

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
    Reserved,
};

enum class MessageId : uint16_t {
    UbxNavPvt = 0x0107,
};

class Message;
class Parser;
class UbloxReceiver {
public:
    explicit UbloxReceiver(Port port, interface::Interface& interface);
    ~UbloxReceiver();

    /// @brief Enable message to be sent by the receiver periodically. This will only take effect
    /// after the next call to configure().
    /// @param message_id The message to enable.
    void enable_message(MessageId message_id);

    /// @brief Disable message to be sent by the receiver periodically. This will only take effect
    /// after the next call to configure().
    /// @param message_id The message to disable.
    void disable_message(MessageId message_id);

    /// @brief Configure the receiver, i.e. enable/disable messages, set port formats, etc.
    void configure();

    /// @brief Read bytes from the interface and append them to the parse buffer. This will not
    /// block.
    void process();

    /// @brief  Block until a message is available.
    /// @return A pointer to the message, or nullptr if the interface is closed.
    UBLOX_NODISCARD Message* wait_for_message();

    /// @brief  Try to parse a message in the parse buffer. This will not block.
    /// @return A pointer to the message, or nullptr if no message is available.
    UBLOX_NODISCARD Message* try_parse();

private:
    Port                  mPort;
    interface::Interface& mInterface;
    Parser*               mParser;
};

}  // namespace ublox
}  // namespace receiver
