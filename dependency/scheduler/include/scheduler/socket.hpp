#pragma once
#include <scheduler/scheduler.hpp>
#include <scheduler/timeout.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <sys/socket.h>

namespace scheduler {

class ListenerTask {
public:
    ListenerTask(int listener_fd, std::string name,
                 std::function<void(ListenerTask&, int, struct sockaddr_storage*, socklen_t)>
                     on_accept) NOEXCEPT;
    ~ListenerTask() NOEXCEPT;

    ListenerTask(ListenerTask const&)            = delete;
    ListenerTask& operator=(ListenerTask const&) = delete;

    void           cancel() NOEXCEPT;
    NODISCARD bool is_scheduled() const NOEXCEPT { return mEvent.valid(); }

    NODISCARD int fd() const NOEXCEPT { return mListenerFd; }

    std::function<void(ListenerTask&)> on_error;

private:
    void on_event(EventInterest triggered) NOEXCEPT;

    ScheduledEvent mEvent;
    int            mListenerFd;

    std::function<void(ListenerTask&, int, struct sockaddr_storage*, socklen_t)> mOnAccept;
};

class SocketListenerTask {
public:
    virtual ~SocketListenerTask() NOEXCEPT;

    void           schedule(Scheduler& scheduler) NOEXCEPT;
    void           schedule() NOEXCEPT { schedule(current()); }
    void           cancel() NOEXCEPT;
    NODISCARD bool is_scheduled() const NOEXCEPT {
        return mListenerTask && mListenerTask->is_scheduled();
    }

    NODISCARD uint16_t port() const NOEXCEPT;
    NODISCARD int      fd() const NOEXCEPT { return mListenerFd; }

    std::function<void(SocketListenerTask&, int, struct sockaddr_storage*, socklen_t)> on_accept;
    std::function<void(SocketListenerTask&)>                                           on_error;

protected:
    SocketListenerTask() NOEXCEPT;

    virtual bool        create_socket() NOEXCEPT    = 0;
    virtual std::string event_name() const NOEXCEPT = 0;

    int                           mListenerFd;
    uint16_t                      mPort;
    std::unique_ptr<ListenerTask> mListenerTask;
};

class TcpInetListenerTask : public SocketListenerTask {
public:
    TcpInetListenerTask(std::string address, uint16_t port) NOEXCEPT;

protected:
    bool        create_socket() NOEXCEPT OVERRIDE;
    std::string event_name() const NOEXCEPT OVERRIDE;

private:
    std::string mAddress;
};

class TcpUnixListenerTask : public SocketListenerTask {
public:
    EXPLICIT TcpUnixListenerTask(std::string path) NOEXCEPT;

protected:
    bool        create_socket() NOEXCEPT OVERRIDE;
    std::string event_name() const NOEXCEPT OVERRIDE;

private:
    std::string mPath;
};

using TcpListenerTask = SocketListenerTask;

class UdpSocketListenerTask {
public:
    virtual ~UdpSocketListenerTask() NOEXCEPT;

    void           schedule(Scheduler& scheduler) NOEXCEPT;
    void           schedule() NOEXCEPT { schedule(current()); }
    void           cancel() NOEXCEPT;
    NODISCARD bool is_scheduled() const NOEXCEPT { return mEvent.valid(); }

    NODISCARD int      fd() const NOEXCEPT { return mListenerFd; }
    NODISCARD uint16_t port() const NOEXCEPT { return mPort; }

    std::function<void(UdpSocketListenerTask&)> on_read;
    std::function<void(UdpSocketListenerTask&)> on_error;

protected:
    UdpSocketListenerTask() NOEXCEPT;

    virtual bool        create_socket() NOEXCEPT    = 0;
    virtual std::string event_name() const NOEXCEPT = 0;

    int            mListenerFd;
    uint16_t       mPort;
    ScheduledEvent mEvent;
};

class UdpInetListenerTask : public UdpSocketListenerTask {
public:
    UdpInetListenerTask(std::string address, uint16_t port) NOEXCEPT;

protected:
    bool        create_socket() NOEXCEPT OVERRIDE;
    std::string event_name() const NOEXCEPT OVERRIDE;

private:
    std::string mAddress;
};

class UdpUnixListenerTask : public UdpSocketListenerTask {
public:
    EXPLICIT UdpUnixListenerTask(std::string path) NOEXCEPT;

protected:
    bool        create_socket() NOEXCEPT OVERRIDE;
    std::string event_name() const NOEXCEPT OVERRIDE;

private:
    std::string mPath;
};

using UdpListenerTask = UdpSocketListenerTask;

class TcpConnectTask {
public:
    EXPLICIT TcpConnectTask(std::string host, uint16_t port, bool should_reconnect) NOEXCEPT;
    EXPLICIT TcpConnectTask(std::string path, bool should_reconnect) NOEXCEPT;
    ~TcpConnectTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    NODISCARD bool schedule() NOEXCEPT { return schedule(current()); }
    bool           cancel() NOEXCEPT;
    NODISCARD bool is_scheduled() const NOEXCEPT { return mIsScheduled; }

    NODISCARD int fd() const NOEXCEPT { return mFd; }

    void set_reconnect_delay(std::chrono::milliseconds delay) NOEXCEPT;

    void update_interests(EventInterest interests) NOEXCEPT;

    std::function<void(TcpConnectTask&)> on_connected;
    std::function<void(TcpConnectTask&)> on_disconnected;
    std::function<void(TcpConnectTask&)> on_read;
    std::function<void(TcpConnectTask&)> on_write;

protected:
    void event(EventInterest triggered) NOEXCEPT;
    bool connect() NOEXCEPT;
    void disconnect() NOEXCEPT;

    void read() NOEXCEPT;
    void write() NOEXCEPT;
    void error() NOEXCEPT;

    enum State {
        StateUnscheduled,
        StateConnecting,
        StateConnected,
        StateDisconnected,
        StateError,
    };

    NODISCARD char const* state_to_string(State state) const NOEXCEPT;

    State                   mState;
    Scheduler*              mScheduler;
    bool                    mIsScheduled;
    ScheduledEvent          mEvent;
    std::string             mEventName;
    int                     mFd;
    std::string             mPath;
    std::string             mHost;
    uint16_t                mPort;
    struct sockaddr_storage mAddress;
    socklen_t               mAddressLength;
    bool                    mConnected;
    bool                    mShouldReconnect;
    RepeatableTimeoutTask   mReconnectTimeout;
};

}  // namespace scheduler
