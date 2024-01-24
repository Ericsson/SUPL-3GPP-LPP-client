#pragma once
#include <atomic>
#include <mutex>
#include <receiver/ublox/receiver.hpp>
#include <receiver/ublox/types.hpp>
#include <receiver/ublox/ubx_nav_pvt.hpp>
#include <thread>

namespace receiver {
namespace ublox {

/// A receiver that run in a separate thread.
class ThreadedReceiver {
public:
    /// The receiver will be created on the thread, thus this will _not_ block.
    UBLOX_EXPLICIT ThreadedReceiver(Port port, std::unique_ptr<interface::Interface> interface,
                                    bool print_messages) UBLOX_NOEXCEPT;
    ~ThreadedReceiver() UBLOX_NOEXCEPT;

    /// Start the receiver thread.
    void start();

    /// Stop the receiver thread.
    void stop();

    /// Interface of the receiver.
    /// @return A pointer to the interface, or nullptr if the receiver is not running.
    UBLOX_NODISCARD interface::Interface* interface() UBLOX_NOEXCEPT;

    /// Get the last received NavPvt message.
    /// @return A unique pointer to the message, or nullptr if no message has been received.
    UBLOX_NODISCARD std::unique_ptr<UbxNavPvt> nav_pvt() UBLOX_NOEXCEPT;

protected:
    /// This function is called at the start of the receiver thread. It handles the blocking
    /// communication with the receiver.
    void run();

private:
    Port                                  mPort;
    std::unique_ptr<interface::Interface> mInterface;
    std::unique_ptr<UbloxReceiver>        mReceiver;
    std::unique_ptr<std::thread>          mThread;
    std::atomic<bool>                     mRunning;
    std::mutex                            mMutex;
    bool                                  mPrintMessages;

    std::unique_ptr<UbxNavPvt> mNavPvt;
};

}  // namespace ublox
}  // namespace receiver
