#pragma once
#include <scheduler/scheduler.hpp>
#include <scheduler/timeout.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <sys/socket.h>

namespace scheduler {

/// Task for listening for incoming connections.
class ListenerTask {
public:
    EXPLICIT ListenerTask(int listener_fd) NOEXCEPT;
    ~ListenerTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    bool           cancel() NOEXCEPT;
    NODISCARD bool is_scheduled() const NOEXCEPT { return mScheduler != nullptr; }

    NODISCARD int fd() const NOEXCEPT { return mListenerFd; }

    std::function<void(ListenerTask&, int, struct sockaddr_storage*, socklen_t)> on_accept;
    std::function<void(ListenerTask&)>                                           on_error;

protected:
    void event(struct epoll_event* event) NOEXCEPT;
    void accept() NOEXCEPT;

private:
    Scheduler* mScheduler;
    EpollEvent mEvent;
    int        mListenerFd;
};

/// Task for listening for incoming TCP connections.
class TcpListenerTask {
public:
    EXPLICIT TcpListenerTask(std::string address, uint16_t port) NOEXCEPT;
    EXPLICIT TcpListenerTask(std::string path) NOEXCEPT;
    ~TcpListenerTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    bool           cancel() NOEXCEPT;
    NODISCARD bool is_scheduled() const NOEXCEPT {
        return mListenerTask && mListenerTask->is_scheduled();
    }

    std::function<void(TcpListenerTask&, int, struct sockaddr_storage*, socklen_t)> on_accept;
    std::function<void(TcpListenerTask&)>                                           on_error;

private:
    std::string                   mPath;
    std::string                   mAddress;
    uint16_t                      mPort;
    std::unique_ptr<ListenerTask> mListenerTask;
};

/// Task for listening for incoming UDP packets.
class UdpListenerTask {
public:
    EXPLICIT UdpListenerTask(std::string address, uint16_t port) NOEXCEPT;
    EXPLICIT UdpListenerTask(std::string path) NOEXCEPT;
    ~UdpListenerTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    bool           cancel() NOEXCEPT;
    NODISCARD bool is_scheduled() const NOEXCEPT { return mScheduler != nullptr; }

    NODISCARD int fd() const NOEXCEPT { return mListenerFd; }

    std::function<void(UdpListenerTask&)> on_read;
    std::function<void(UdpListenerTask&)> on_error;

private:
    void event(struct epoll_event* event) NOEXCEPT;

    std::string mPath;
    std::string mAddress;
    uint16_t    mPort;
    Scheduler*  mScheduler;
    EpollEvent  mEvent;
    int         mListenerFd;
};

/// Task for reading and writing to a socket.
class SocketTask {
public:
    EXPLICIT SocketTask(int fd) NOEXCEPT;
    ~SocketTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    bool           cancel() NOEXCEPT;
    NODISCARD bool is_scheduled() const NOEXCEPT { return mScheduler != nullptr; }

    NODISCARD int fd() const NOEXCEPT { return mFd; }

    std::function<void(SocketTask&)> on_read;
    std::function<void(SocketTask&)> on_write;
    std::function<void(SocketTask&)> on_error;

protected:
    void event(struct epoll_event* event) NOEXCEPT;
    void read() NOEXCEPT;
    void write() NOEXCEPT;
    void error() NOEXCEPT;

private:
    Scheduler* mScheduler;
    EpollEvent mEvent;
    int        mFd;
};

class TcpConnectTask {
public:
    EXPLICIT TcpConnectTask(std::string host, uint16_t port, bool should_reconnect) NOEXCEPT;
    EXPLICIT TcpConnectTask(std::string path, bool should_reconnect) NOEXCEPT;
    ~TcpConnectTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    bool           cancel() NOEXCEPT;
    NODISCARD bool is_scheduled() const NOEXCEPT { return mIsScheduled; }

    NODISCARD int fd() const NOEXCEPT { return mFd; }

    std::function<void(TcpConnectTask&)> on_connected;
    std::function<void(TcpConnectTask&)> on_disconnected;
    std::function<void(TcpConnectTask&)> on_read;
    std::function<void(TcpConnectTask&)> on_write;

protected:
    void event(struct epoll_event* event) NOEXCEPT;
    bool connect() NOEXCEPT;
    void disconnect() NOEXCEPT;

    void read() NOEXCEPT;
    void write() NOEXCEPT;
    void error() NOEXCEPT;

    enum State {
        STATE_UNSCEDULED,
        STATE_CONNECTING,
        STATE_CONNECTED,
        STATE_DISCONNECTED,
        STATE_ERROR,
    };

    char const* state_to_string(State state) const NOEXCEPT;

    State                   mState;
    Scheduler*              mScheduler;
    bool                    mIsScheduled;
    EpollEvent              mEvent;
    int                     mFd;
    std::string             mPath;
    std::string             mHost;
    uint16_t                mPort;
    struct sockaddr_storage mAddress;
    socklen_t               mAddressLength;
    bool                    mConnected;
    bool                    mShouldReconnect;
    TimeoutTask             mReconnectTimeout;
};

}  // namespace scheduler
