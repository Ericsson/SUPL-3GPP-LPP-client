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

ThreadedReceiver::ThreadedReceiver(std::unique_ptr<interface::Interface> interface,
                                   bool                                  print_messages,
                                   std::unique_ptr<std::string> export_socket) NMEA_NOEXCEPT
    : mInterface(std::move(interface)),
      mRunning(false),
      mPrintMessages(print_messages),
      mExportInterface(nullptr),
      mGga(nullptr),
      mVtg(nullptr),
      mGst(nullptr) {
    RNT_DEBUG("[rnt] created\n");

    if (export_socket) {
        mExportInterface = std::unique_ptr<interface::Interface>(
            interface::Interface::file(*export_socket, false));
        printf("[export interface]\n");
        mExportInterface->open();
        mExportInterface->print_info();
    }
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

                for (;;) {
                    RNT_DEBUG("[rnt] check\n");
                    auto message = mReceiver->try_parse();
                    if (message) {
                        if (mPrintMessages) {
                            message->print();
                        }
                        if (dynamic_cast<GgaMessage*>(message.get())) {
                            mGga = std::unique_ptr<GgaMessage>(
                                static_cast<GgaMessage*>(message.release()));
                        } else if (dynamic_cast<VtgMessage*>(message.get())) {
                            mVtg = std::unique_ptr<VtgMessage>(
                                static_cast<VtgMessage*>(message.release()));
                        } else if (dynamic_cast<GstMessage*>(message.get())) {
                            mGst = std::unique_ptr<GstMessage>(
                                static_cast<GstMessage*>(message.release()));
                        }
                        if (mExportInterface) {
                            if (mExportInterface->can_write()) {
                                auto message_data = message->sentence();
                                mExportInterface->write(message_data.data(), message_data.size());
                            }
                        }
                    } else {
                        break;
                    }
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

std::unique_ptr<GgaMessage> ThreadedReceiver::gga() NMEA_NOEXCEPT {
    if (!mReceiver) return nullptr;
    RNT_DEBUG("[rnt] lock (gga)\n");
    std::lock_guard<std::mutex> lock(mMutex);

    if (!mGga) return nullptr;
    auto gga = std::unique_ptr<GgaMessage>(new GgaMessage{*mGga.get()});
    RNT_DEBUG("[rnt] unlock (gga)\n");
    return gga;
}

std::unique_ptr<VtgMessage> ThreadedReceiver::vtg() NMEA_NOEXCEPT {
    if (!mReceiver) return nullptr;
    RNT_DEBUG("[rnt] lock (vtg)\n");
    std::lock_guard<std::mutex> lock(mMutex);

    if (!mVtg) return nullptr;
    auto vtg = std::unique_ptr<VtgMessage>(new VtgMessage{*mVtg.get()});
    RNT_DEBUG("[rnt] unlock (vtg)\n");
    return vtg;
}

std::unique_ptr<GstMessage> ThreadedReceiver::gst() NMEA_NOEXCEPT {
    if (!mReceiver) return nullptr;
    RNT_DEBUG("[rnt] lock (gst)\n");
    std::lock_guard<std::mutex> lock(mMutex);

    if (!mGst) return nullptr;
    auto gst = std::unique_ptr<GstMessage>(new GstMessage{*mGst.get()});
    RNT_DEBUG("[rnt] unlock (gst)\n");
    return gst;
}

}  // namespace nmea
}  // namespace receiver
