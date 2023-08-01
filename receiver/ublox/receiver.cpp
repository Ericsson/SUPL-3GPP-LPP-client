#include "receiver.hpp"
#include <interface/interface.hpp>
#include <stdexcept>
#include "encoder.hpp"
#include "message.hpp"
#include "parser.hpp"
#include "ubx_mon_ver.hpp"

namespace receiver {
namespace ublox {

UbloxReceiver::UbloxReceiver(Port port, interface::Interface& interface)
    : mPort(port), mInterface(interface) {
    mParser = new Parser();
}

UbloxReceiver::~UbloxReceiver() {
    if (mParser != nullptr) {
        delete mParser;
    }
}

void UbloxReceiver::enable_message(MessageId message_id) {}

void UbloxReceiver::disable_message(MessageId message_id) {}

void UbloxReceiver::configure() {
    uint8_t mon_ver_buffer[64];

    auto encoder             = Encoder{mon_ver_buffer, sizeof(mon_ver_buffer)};
    auto poll_message_length = UbxMonVer::poll(encoder);
    mInterface.write(mon_ver_buffer, poll_message_length);
}

void UbloxReceiver::process() {
    if (mInterface.can_read()) {
        uint8_t buffer[256];
        auto    length = mInterface.read(buffer, sizeof(buffer));
        if (length <= 0) {
            throw std::runtime_error("Failed to read from interface");
        }

        if (!mParser->append(buffer, length)) {
            throw std::runtime_error("Failed to append to parser");
        }
    }
}

Message* UbloxReceiver::wait_for_message() {
    for (;;) {
        process();

        auto message = try_parse();
        if (message) {
            return message;
        }
    }
}

Message* UbloxReceiver::try_parse() {
    return mParser->try_parse();
}

}  // namespace ublox
}  // namespace receiver
