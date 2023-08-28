#include "threaded_receiver.hpp"
#include <interface/interface.hpp>
#include <unistd.h>
#include "message.hpp"

namespace receiver {
namespace ublox {

ThreadedReceiver::ThreadedReceiver(Port                                  port,
                                   std::unique_ptr<interface::Interface> interface) UBLOX_NOEXCEPT
    : mPort(port),
      mInterface(std::move(interface)),
      mRunning(false) {}

ThreadedReceiver::~ThreadedReceiver() UBLOX_NOEXCEPT {
    stop();
}

void ThreadedReceiver::start() {
    if (mRunning) return;
    mRunning = true;
    mThread  = std::unique_ptr<std::thread>(new std::thread(&ThreadedReceiver::run, this));
}

void ThreadedReceiver::stop() {
    if (mRunning) {
        mRunning = false;
        if (mThread && mThread->joinable()) {
            mThread->join();
        }
    }
}

void ThreadedReceiver::run() {
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mReceiver = std::unique_ptr<UbloxReceiver>(new UbloxReceiver(mPort, std::move(mInterface)));
    }

    while (mRunning) {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            auto                        message = mReceiver->wait_for_message();
            if (message) {
                if (message->message_class() == UbxNavPvt::CLASS_ID &&
                    message->message_id() == UbxNavPvt::MESSAGE_ID) {
                    // Cast unique_ptr<Message> to unique_ptr<UbxNavPvt>.
                    mNavPvt =
                        std::unique_ptr<UbxNavPvt>(static_cast<UbxNavPvt*>(message.release()));
                }
            }
        }

        usleep(10 * 1000);
    }
}

}  // namespace ublox
}  // namespace receiver