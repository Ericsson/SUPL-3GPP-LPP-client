#pragma once
#include <io/input.hpp>
#include <io/output.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <sys/socket.h>
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
    EXPLICIT TcpServerInput(std::string path) NOEXCEPT;
    ~TcpServerInput() NOEXCEPT override;

    NODISCARD std::string const& listen() const NOEXCEPT { return mListen; }
    NODISCARD uint16_t           port() const NOEXCEPT { return mPort; }
    NODISCARD std::string const& path() const NOEXCEPT { return mPath; }

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::string mPath;
    std::string mListen;
    uint16_t    mPort;

    std::unique_ptr<scheduler::TcpListenerTask>         mListenerTask;
    std::vector<std::unique_ptr<scheduler::SocketTask>> mClientTasks;

    uint8_t mBuffer[4096];
};

/// An input that connects to a TCP server and reads from it.
class TcpClientInput : public Input {
public:
    EXPLICIT TcpClientInput(std::string host, uint16_t port, bool reconnect) NOEXCEPT;
    EXPLICIT TcpClientInput(std::string path, bool reconnect) NOEXCEPT;
    ~TcpClientInput() NOEXCEPT override;

    NODISCARD std::string const& host() const NOEXCEPT { return mHost; }
    NODISCARD uint16_t           port() const NOEXCEPT { return mPort; }
    NODISCARD std::string const& path() const NOEXCEPT { return mPath; }
    NODISCARD bool               reconnect() const NOEXCEPT { return mReconnect; }

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::string mPath;
    std::string mHost;
    uint16_t    mPort;
    bool        mReconnect;

    std::unique_ptr<scheduler::TcpConnectTask> mConnectTask;

    uint8_t mBuffer[4096];
};

/// An output that connects to a TCP server and writes to it.
class TcpClientOutput : public Output {
public:
    EXPLICIT TcpClientOutput(std::string host, uint16_t port, bool reconnect) NOEXCEPT;
    EXPLICIT TcpClientOutput(std::string path, bool reconnect) NOEXCEPT;
    ~TcpClientOutput() NOEXCEPT override;

    NODISCARD const char* name() const NOEXCEPT override { return "tcp-client"; }

    void write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::string const& host() const NOEXCEPT { return mHost; }
    NODISCARD uint16_t           port() const NOEXCEPT { return mPort; }
    NODISCARD std::string const& path() const NOEXCEPT { return mPath; }
    NODISCARD bool               reconnect() const NOEXCEPT { return mReconnect; }

protected:
    enum State {
        StateInitial,
        StateConnecting,
        StateConnected,
        StateDisconnected,
        StateError,
        StateReconnect,
    };

    bool connect() NOEXCEPT;
    bool connecting() NOEXCEPT;
    void disconnect() NOEXCEPT;

    char const* state_to_string(State state) const NOEXCEPT;

private:
    State                                 mState;
    std::string                           mHost;
    uint16_t                              mPort;
    std::string                           mPath;
    bool                                  mReconnect;
    int                                   mFd;
    struct sockaddr_storage               mAddress;
    socklen_t                             mAddressLength;
    std::chrono::steady_clock::time_point mReconnectTime;
};

class TcpServerOutput : public Output {
public:
    EXPLICIT TcpServerOutput(std::string listen, uint16_t port) NOEXCEPT;
    EXPLICIT TcpServerOutput(std::string path) NOEXCEPT;
    ~TcpServerOutput() NOEXCEPT override;

    NODISCARD const char* name() const NOEXCEPT override { return "tcp-server"; }

    void write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::string const& listen() const NOEXCEPT { return mListen; }
    NODISCARD uint16_t           port() const NOEXCEPT { return mPort; }
    NODISCARD std::string const& path() const NOEXCEPT { return mPath; }

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

    void remove_client(int fd) NOEXCEPT;

private:
    std::string mPath;
    std::string mListen;
    uint16_t    mPort;

    std::unique_ptr<scheduler::TcpListenerTask>         mListenerTask;
    std::vector<std::unique_ptr<scheduler::SocketTask>> mClientTasks;
    std::vector<std::unique_ptr<scheduler::SocketTask>> mRemoveClientTasks;
};

}  // namespace io
