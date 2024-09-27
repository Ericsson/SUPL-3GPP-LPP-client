#pragma once
#include <io/input.hpp>

#include <memory>
#include <string>
#include <vector>

namespace scheduler {
class UdpListenerTask;
}  // namespace scheduler

namespace io {
/// An input that binds to a UDP port and reads from it.
class UdpServerInput : public Input {
public:
    EXPLICIT UdpServerInput(std::string listen, uint16_t port) NOEXCEPT;
    ~UdpServerInput() NOEXCEPT override;

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::string mListen;
    uint16_t    mPort;

    std::unique_ptr<scheduler::UdpListenerTask> mListenerTask;
    uint8_t                                     mBuffer[4096];
};
}  // namespace io
