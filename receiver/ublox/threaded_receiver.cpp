#include "threaded_receiver.hpp"
#include <interface/interface.hpp>
#include <unistd.h>
#include "message.hpp"

#ifdef RECEIVER_UBLOX_THREADED
#include <cstdio>
#define RUT_DEBUG(...) printf(__VA_ARGS__)
#else
#define RUT_DEBUG(...)
#endif

namespace receiver {
namespace ublox {

ThreadedReceiver::ThreadedReceiver(Port port, std::unique_ptr<interface::Interface> interface,
                                   bool print_messages) UBLOX_NOEXCEPT
    : mPort(port),
      mInterface(std::move(interface)),
      mRunning(false),
      mPrintMessages(print_messages) {
    RUT_DEBUG("[rut] created\n");
}

ThreadedReceiver::~ThreadedReceiver() UBLOX_NOEXCEPT {
    stop();
    RUT_DEBUG("[rut] destroyed\n");
}

void ThreadedReceiver::start() {
    if (mRunning) return;
    RUT_DEBUG("[rut] starting\n");
    mRunning = true;
    mThread  = std::unique_ptr<std::thread>(new std::thread(&ThreadedReceiver::run, this));
}

void ThreadedReceiver::stop() {
    if (mRunning) {
        RUT_DEBUG("[rut] stopping\n");
        mRunning = false;
        if (mThread && mThread->joinable()) {
            RUT_DEBUG("[rut] joining\n");
            mThread->join();
        }
    }
}

void ThreadedReceiver::run() {
    RUT_DEBUG("[rut] running\n");

    {
        RUT_DEBUG("[rut] lock (run)\n");
        std::lock_guard<std::mutex> lock(mMutex);
        RUT_DEBUG("[rut] create receiver\n");
        mReceiver = std::unique_ptr<UbloxReceiver>(new UbloxReceiver(mPort, std::move(mInterface)));
        RUT_DEBUG("[rut] unlock (run)\n");
    }

    while (mRunning) {
        {
            if (mReceiver) {
                RUT_DEBUG("[rut] process\n");
                mReceiver->process();

                for (;;) {
                    RUT_DEBUG("[rut] check\n");
                    auto message = mReceiver->try_parse();
                    if (message) {
                        RUT_DEBUG("[rut] lock (run)\n");
                        std::lock_guard<std::mutex> lock(mMutex);

                        if (mPrintMessages) {
                            message->print();
                        }
                        if (message->message_class() == UbxNavPvt::CLASS_ID &&
                            message->message_id() == UbxNavPvt::MESSAGE_ID) {
                            RUT_DEBUG("[rut] save navpvt\n");
                            // Cast unique_ptr<Message> to unique_ptr<UbxNavPvt>.
                            mNavPvt = std::unique_ptr<UbxNavPvt>(
                                static_cast<UbxNavPvt*>(message.release()));
                        }
                        RUT_DEBUG("[rut] unlock (run)\n");
                    } else {
                        break;
                    }
                }
            }
        }

        if (mReceiver) {
            RUT_DEBUG("[rut] wait\n");
            mReceiver->interface().wait_for_read();
        }

        RUT_DEBUG("[rut] sleep\n");
        usleep(10 * 1000);
    }
}

interface::Interface* ThreadedReceiver::interface() UBLOX_NOEXCEPT {
    if (!mReceiver) return nullptr;
    RUT_DEBUG("[rut] lock (interface)\n");
    std::lock_guard<std::mutex> lock(mMutex);
    auto                        interface = &mReceiver->interface();
    RUT_DEBUG("[rut] unlock (interface)\n");
    return interface;
}

std::unique_ptr<UbxNavPvt> ThreadedReceiver::nav_pvt() UBLOX_NOEXCEPT {
    if (!mReceiver) return nullptr;
    RUT_DEBUG("[rut] lock (nav_pvt)\n");
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mNavPvt) return nullptr;
    // Copy mNavPvt to avoid locking the mutex for too long.
    auto nav_pvt = std::unique_ptr<UbxNavPvt>(new UbxNavPvt(*mNavPvt));
    RUT_DEBUG("[rut] unlock (nav_pvt)\n");
    return nav_pvt;
}

}  // namespace ublox
}  // namespace receiver