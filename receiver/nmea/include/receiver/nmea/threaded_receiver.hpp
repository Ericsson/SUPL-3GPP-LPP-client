#pragma once
#include <receiver/nmea/receiver.hpp>
#include <receiver/nmea/types.hpp>

#include <atomic>
#include <mutex>
#include <thread>

namespace receiver {
namespace nmea {

/// A receiver that run in a separate thread.
class ThreadedReceiver {
public:
    /// The receiver will be created on the thread, thus this will _not_ block.
    NMEA_EXPLICIT ThreadedReceiver(std::unique_ptr<interface::Interface> interface) NMEA_NOEXCEPT;
    ~ThreadedReceiver() NMEA_NOEXCEPT;

    /// Start the receiver thread.
    void start();

    /// Stop the receiver thread.
    void stop();

    /// Interface of the receiver.
    /// @return A pointer to the interface, or nullptr if the receiver is not running.
    NMEA_NODISCARD interface::Interface* interface() NMEA_NOEXCEPT;

protected:
    /// This function is called at the start of the receiver thread. It handles the blocking
    /// communication with the receiver.
    void run();

private:
    std::unique_ptr<interface::Interface> mInterface;
    std::unique_ptr<NmeaReceiver>         mReceiver;
    std::unique_ptr<std::thread>          mThread;
    std::atomic<bool>                     mRunning;
    std::mutex                            mMutex;
};

}  // namespace nmea
}  // namespace receiver
