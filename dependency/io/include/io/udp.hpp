#pragma once
#include <io/input.hpp>
#include <io/output.hpp>

#include <memory>
#include <string>
#include <sys/socket.h>
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

/// An output that sends UDP packets to a server.
class UdpClientOutput : public Output {
public:
    EXPLICIT UdpClientOutput(std::string host, uint16_t port) NOEXCEPT;
    EXPLICIT UdpClientOutput(std::string path) NOEXCEPT;
    ~UdpClientOutput() NOEXCEPT override;

    void write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::string const& host() const NOEXCEPT { return mHost; }
    NODISCARD uint16_t           port() const NOEXCEPT { return mPort; }
    NODISCARD std::string const& path() const NOEXCEPT { return mPath; }

    void open() NOEXCEPT;
    void close() NOEXCEPT;

private:
    std::string             mHost;
    uint16_t                mPort;
    std::string             mPath;
    int                     mFd;
    struct sockaddr_storage mAddress;
    socklen_t               mAddressLength;
};

}  // namespace io
