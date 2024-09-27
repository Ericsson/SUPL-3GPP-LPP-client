#pragma once
#include <io/input.hpp>

#include <memory>
#include <string>
#include <vector>

namespace scheduler {
class TcpListenerTask;
class SocketTask;
class TcpConnectTask;
}  // namespace scheduler

namespace io {
/// An input that accepts TCP connections and reads from them.
class TcpServerInput : public Input {
public:
    EXPLICIT TcpServerInput(std::string listen, uint16_t port) NOEXCEPT;
    ~TcpServerInput() NOEXCEPT override;

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::string mListen;
    uint16_t    mPort;

    std::unique_ptr<scheduler::TcpListenerTask>         mListenerTask;
    std::vector<std::unique_ptr<scheduler::SocketTask>> mClientTasks;

    uint8_t mBuffer[4096];
};

/// An input that connects to a TCP server and reads from it.
class TcpClientInput : public Input {
public:
    EXPLICIT TcpClientInput(std::string host, uint16_t port) NOEXCEPT;
    ~TcpClientInput() NOEXCEPT override;

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::string mHost;
    uint16_t    mPort;

    std::unique_ptr<scheduler::TcpConnectTask> mConnectTask;

    uint8_t mBuffer[4096];
};
}  // namespace io
