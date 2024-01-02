#include "threaded_receiver.hpp"
#include <interface/interface.hpp>
#include <unistd.h>
#include "message.hpp"

#ifdef RECEIVER_NMEA_THREADED
#include <cstdio>
#define RNT_DEBUG(...) printf(__VA_ARGS__)
#else
#define RNT_DEBUG(...)
#endif

namespace receiver {
namespace nmea {

ThreadedReceiver::ThreadedReceiver(std::unique_ptr<interface::Interface> interface) NMEA_NOEXCEPT
    : mInterface(std::move(interface)),
      mRunning(false) {
    RNT_DEBUG("[rnt] created\n");
}

ThreadedReceiver::~ThreadedReceiver() NMEA_NOEXCEPT {
    stop();
    RNT_DEBUG("[rnt] destroyed\n");
}

void ThreadedReceiver::start() {
    if (mRunning) return;
    RNT_DEBUG("[rnt] starting\n");
    mRunning = true;
    mThread  = std::unique_ptr<std::thread>(new std::thread(&ThreadedReceiver::run, this));
}

void ThreadedReceiver::stop() {
    if (mRunning) {
        RNT_DEBUG("[rnt] stopping\n");
        mRunning = false;
        if (mThread && mThread->joinable()) {
            RNT_DEBUG("[rnt] joining\n");
            mThread->join();
        }
    }
}

void ThreadedReceiver::run() {
    RNT_DEBUG("[rnt] running\n");

    {
        RNT_DEBUG("[rnt] lock (run)\n");
        std::lock_guard<std::mutex> lock(mMutex);
        RNT_DEBUG("[rnt] create receiver\n");
        mReceiver = std::unique_ptr<NmeaReceiver>(new NmeaReceiver(std::move(mInterface)));
        RNT_DEBUG("[rnt] unlock (run)\n");
    }

    while (mRunning) {
        {
            RNT_DEBUG("[rnt] lock (run)\n");
            std::lock_guard<std::mutex> lock(mMutex);

            if (mReceiver) {
                RNT_DEBUG("[rnt] process\n");
                mReceiver->process();

                RNT_DEBUG("[rnt] check\n");
                auto message = mReceiver->try_parse();
                if (message) {
                    // TODO:
                }
            }
            RNT_DEBUG("[rnt] unlock (run)\n");
        }

        if (mReceiver) {
            RNT_DEBUG("[rnt] wait\n");
            mReceiver->interface().wait_for_read();
        }

        RNT_DEBUG("[rnt] sleep\n");
        usleep(10 * 1000);
    }
}

interface::Interface* ThreadedReceiver::interface() NMEA_NOEXCEPT {
    if (!mReceiver) return nullptr;
    RNT_DEBUG("[rnt] lock (interface)\n");
    std::lock_guard<std::mutex> lock(mMutex);
    auto                        interface = &mReceiver->interface();
    RNT_DEBUG("[rnt] unlock (interface)\n");
    return interface;
}

}  // namespace nmea
}  // namespace receiver