#pragma once
#include <memory>
#include <receiver/nmea/types.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace interface {
class Interface;
}

namespace receiver {
namespace nmea {

class Message;
class Parser;
class NmeaReceiver {
public:
    /// Construct a receiver. This will block until the receiver configuration has been
    /// acquired.
    NMEA_EXPLICIT NmeaReceiver(std::unique_ptr<interface::Interface> interface) NMEA_NOEXCEPT;
    ~NmeaReceiver() NMEA_NOEXCEPT;

    /// Read bytes from the interface and append them to the parse buffer. This will not
    /// block.
    void process();

    ///  Block until a message is available.
    /// @return A pointer to the message, or nullptr if the interface is closed.
    NMEA_NODISCARD std::unique_ptr<Message> wait_for_message();

    ///  Try to parse a message in the parse buffer. This will not block.
    /// @return A pointer to the message, or nullptr if no message is available.
    NMEA_NODISCARD std::unique_ptr<Message> try_parse();

    /// The receiver interface.
    /// @return The receiver interface.
    NMEA_NODISCARD interface::Interface& interface() const NMEA_NOEXCEPT { return *mInterface; }

protected:
private:
    std::unique_ptr<interface::Interface> mInterface;
    Parser*                               mParser;
};

}  // namespace nmea
}  // namespace receiver
