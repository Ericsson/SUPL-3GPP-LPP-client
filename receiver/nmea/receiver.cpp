#include "receiver.hpp"
#include "message.hpp"
#include "parser.hpp"

#include <cinttypes>
#include <interface/interface.hpp>
#include <stdexcept>

namespace receiver {
namespace nmea {

NmeaReceiver::NmeaReceiver(std::unique_ptr<interface::Interface> interface) NMEA_NOEXCEPT
    : mInterface(std::move(interface)) {
    mParser = new Parser();
}

NmeaReceiver::~NmeaReceiver() NMEA_NOEXCEPT {
    if (mParser != nullptr) {
        delete mParser;
    }
}

void NmeaReceiver::process() {
    if (!mInterface->is_open()) {
        mInterface->open();
    }

    if (mInterface->can_read()) {
        uint8_t buffer[1024];
        auto    available = mParser->available_space();
        if (available > 0) {
            auto read_length = (available > sizeof(buffer)) ? sizeof(buffer) : available;
            auto length      = mInterface->read(buffer, read_length);
            if (length <= 0) {
                // This will only happen if the interface is closed or disconnected. In this case,
                // we will try to re-open the interface in the next iteration of the main loop.
                mInterface->close();
                return;
            }

            // Ignore the return value, this should never fail if the parser buffer is >= 1024 bytes
            mParser->append(buffer, length);
        }
    }
}

std::unique_ptr<Message> NmeaReceiver::wait_for_message() {
    for (;;) {
        process();

        auto message = try_parse();
        if (message) {
            return message;
        }

        mInterface->wait_for_read();
        if (!mInterface->is_open() && mParser->buffer_length() == 0) {
            return nullptr;
        }
    }
}

std::unique_ptr<Message> NmeaReceiver::try_parse() {
    return mParser->try_parse();
}

}  // namespace nmea
}  // namespace receiver
