#include "socket.hpp"
#include "epoll_constants.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(sched, socket);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(sched, socket)

namespace scheduler {

ListenerTask::ListenerTask(
    int listener_fd, std::string name,
    std::function<void(ListenerTask&, int, struct sockaddr_storage*, socklen_t)> on_accept) NOEXCEPT
    : mEvent{ScheduledEvent::invalid()},
      mListenerFd{listener_fd},
      mOnAccept{std::move(on_accept)} {
    VSCOPE_FUNCTION();

    if (mListenerFd < 0) {
        ERRORF("invalid file descriptor");
        return;
    }

    auto fcntl_flags = ::fcntl(mListenerFd, F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", mListenerFd, fcntl_flags);
    if (!(fcntl_flags & O_NONBLOCK)) {
        ERRORF("listener fd must be non-blocking");
        return;
    }

    auto result = ::listen(mListenerFd, SOMAXCONN);
    VERBOSEF("::listen(%d, %d) = %d", mListenerFd, SOMAXCONN, result);
    if (result < 0) {
        ERRORF("listen failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        return;
    }

    mEvent = current().register_fd(
        mListenerFd, EventInterest::Read | EventInterest::Error | EventInterest::Hangup,
        [this](EventInterest i) {
            on_event(i);
        },
        std::move(name));
}

ListenerTask::~ListenerTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

void ListenerTask::on_event(EventInterest triggered) NOEXCEPT {
    if (triggered & (EventInterest::Error | EventInterest::Hangup)) {
        if (on_error) on_error(*this);
        return;
    }

    if (triggered & EventInterest::Read) {
        struct sockaddr_storage addr{};
        socklen_t               addr_len = sizeof(addr);
        auto client_fd = ::accept(mListenerFd, (struct sockaddr*)&addr, &addr_len);
        VERBOSEF("::accept(%d, %p, %p) = %d", mListenerFd, &addr, &addr_len, client_fd);
        if (client_fd >= 0) {
            auto flags = ::fcntl(client_fd, F_GETFL, 0);
            VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", client_fd, flags);
            auto result = ::fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
            VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", client_fd, flags | O_NONBLOCK, result);
            mOnAccept(*this, client_fd, &addr, addr_len);
        } else {
            WARNF("accept failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        }
    }
}

void ListenerTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();
    mEvent.unregister();
}

//
//
//

SocketListenerTask::SocketListenerTask() NOEXCEPT : mListenerFd{-1},
                                                    mPort{0},
                                                    mListenerTask{nullptr} {}

SocketListenerTask::~SocketListenerTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

void SocketListenerTask::schedule(Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    if (mListenerTask) {
        WARNF("already scheduled");
        return;
    }

    if (!create_socket()) return;

    auto fcntl_flags = ::fcntl(mListenerFd, F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", mListenerFd, fcntl_flags);
    auto result = ::fcntl(mListenerFd, F_SETFL, fcntl_flags | O_NONBLOCK);
    VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", mListenerFd, fcntl_flags | O_NONBLOCK, result);

    mListenerTask.reset(
        new ListenerTask(mListenerFd, event_name(),
                         [this](ListenerTask&, int client_fd, struct sockaddr_storage* client_addr,
                                socklen_t client_addr_len) {
                             if (on_accept)
                                 on_accept(*this, client_fd, client_addr, client_addr_len);
                         }));
    mListenerTask->on_error = [this](ListenerTask&) {
        if (on_error) on_error(*this);
    };
}

void SocketListenerTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mListenerTask) {
        mListenerTask->cancel();
        mListenerTask.reset();
    }

    if (mListenerFd >= 0) {
        auto result = ::close(mListenerFd);
        VERBOSEF("::close(%d) = %d", mListenerFd, result);
        mListenerFd = -1;
    }
}

uint16_t SocketListenerTask::port() const NOEXCEPT {
    return mPort;
}

//
// TcpInetListenerTask
//

TcpInetListenerTask::TcpInetListenerTask(std::string address, uint16_t port) NOEXCEPT
    : mAddress{std::move(address)} {
    mPort = port;
}

bool TcpInetListenerTask::create_socket() NOEXCEPT {
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(mPort);
    auto result     = ::inet_pton(AF_INET, mAddress.c_str(), &addr.sin_addr);
    VERBOSEF("::inet_pton(AF_INET, %s, %p) = %d", mAddress.c_str(), &addr.sin_addr, result);
    if (result <= 0) {
        ERRORF("inet_pton failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        return false;
    }

    mListenerFd = ::socket(AF_INET, SOCK_STREAM, 0);
    VERBOSEF("::socket(AF_INET, SOCK_STREAM, 0) = %d", mListenerFd);
    if (mListenerFd < 0) {
        ERRORF("socket failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        return false;
    }

    int enable = 1;
    result     = ::setsockopt(mListenerFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    VERBOSEF("::setsockopt(%d, SOL_SOCKET, SO_REUSEADDR, %p, %zu) = %d", mListenerFd, &enable,
             sizeof(enable), result);

    result = ::bind(mListenerFd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    VERBOSEF("::bind(%d, %p, %zu) = %d", mListenerFd, &addr, sizeof(addr), result);
    if (result < 0) {
        ERRORF("bind failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        result = ::close(mListenerFd);
        VERBOSEF("::close(%d) = %d", mListenerFd, result);
        mListenerFd = -1;
        return false;
    }

    if (mPort == 0) {
        struct sockaddr_in bound_addr{};
        socklen_t          bound_len = sizeof(bound_addr);
        result =
            ::getsockname(mListenerFd, reinterpret_cast<struct sockaddr*>(&bound_addr), &bound_len);
        VERBOSEF("::getsockname(%d, %p, %p) = %d", mListenerFd, &bound_addr, &bound_len, result);
        if (result == 0) {
            mPort = ntohs(bound_addr.sin_port);
            INFOF("bound to port %u", mPort);
        }
    }

    return true;
}

std::string TcpInetListenerTask::event_name() const NOEXCEPT {
    return "tcp-listener:" + mAddress + ":" + std::to_string(mPort);
}

//
// TcpUnixListenerTask
//

TcpUnixListenerTask::TcpUnixListenerTask(std::string path) NOEXCEPT : mPath{std::move(path)} {}

bool TcpUnixListenerTask::create_socket() NOEXCEPT {
    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;

    if (mPath.size() + 1 >= sizeof(addr.sun_path)) {
        ERRORF("path too long for unix socket: \"%s\"", mPath.c_str());
        return false;
    }

    memset(addr.sun_path, 0, sizeof(addr.sun_path));
    memcpy(addr.sun_path, mPath.c_str(), mPath.size());

    mListenerFd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    VERBOSEF("::socket(AF_UNIX, SOCK_STREAM, 0) = %d", mListenerFd);
    if (mListenerFd < 0) {
        ERRORF("socket failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        return false;
    }

    auto addr_len = static_cast<socklen_t>(sizeof(sa_family_t) + mPath.size() + 1);
    auto result   = ::bind(mListenerFd, reinterpret_cast<struct sockaddr*>(&addr), addr_len);
    VERBOSEF("::bind(%d, %p, %d) = %d", mListenerFd, &addr, addr_len, result);
    if (result < 0) {
        ERRORF("bind failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        result = ::close(mListenerFd);
        VERBOSEF("::close(%d) = %d", mListenerFd, result);
        mListenerFd = -1;
        return false;
    }

    return true;
}

std::string TcpUnixListenerTask::event_name() const NOEXCEPT {
    return "tcp-listener:" + mPath;
}

//
//
//

UdpSocketListenerTask::UdpSocketListenerTask() NOEXCEPT : mListenerFd{-1},
                                                          mPort{0},
                                                          mEvent{ScheduledEvent::invalid()} {}

UdpSocketListenerTask::~UdpSocketListenerTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

void UdpSocketListenerTask::schedule(Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    if (mEvent.valid()) {
        WARNF("already scheduled");
        return;
    }

    if (!create_socket()) {
        return;
    }

    EventInterest interests = EventInterest::Error | EventInterest::Hangup;
    if (on_read) interests = interests | EventInterest::Read;

    mEvent = scheduler.register_fd(
        mListenerFd, interests,
        [this](EventInterest triggered) {
            VERBOSEF("event fd=%d triggered: %s%s%s%s", mListenerFd,
                     (triggered & EventInterest::Read) ? "read " : "",
                     (triggered & EventInterest::Write) ? "write " : "",
                     (triggered & EventInterest::Error) ? "error " : "",
                     (triggered & EventInterest::Hangup) ? "hangup " : "");
            if (triggered & (EventInterest::Error | EventInterest::Hangup)) {
                if (on_error) on_error(*this);
            } else if (triggered & EventInterest::Read) {
                if (on_read) on_read(*this);
            }
        },
        event_name());

    if (!mEvent.valid()) {
        auto result = ::close(mListenerFd);
        VERBOSEF("::close(%d) = %d", mListenerFd, result);
        mListenerFd = -1;
    }
}

void UdpSocketListenerTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();
    mEvent.unregister();

    if (mListenerFd < 0) return;
    auto result = ::close(mListenerFd);
    VERBOSEF("::close(%d) = %d", mListenerFd, result);
    mListenerFd = -1;
}

UdpInetListenerTask::UdpInetListenerTask(std::string address, uint16_t port) NOEXCEPT
    : mAddress{std::move(address)} {
    mPort = port;
    VSCOPE_FUNCTIONF("\"%s\", %u", mAddress.c_str(), mPort);
}

bool UdpInetListenerTask::create_socket() NOEXCEPT {
    struct sockaddr_in addr_in{};
    addr_in.sin_family = AF_INET;
    addr_in.sin_port   = htons(mPort);
    auto result        = ::inet_pton(AF_INET, mAddress.c_str(), &addr_in.sin_addr);
    VERBOSEF("::inet_pton(AF_INET, %s, %p) = %d", mAddress.c_str(), &addr_in.sin_addr, result);
    if (result <= 0) {
        ERRORF("inet_pton failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        return false;
    }

    auto listener_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    VERBOSEF("::socket(AF_INET, SOCK_DGRAM, 0) = %d", listener_fd);
    if (listener_fd < 0) {
        ERRORF("socket failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        return false;
    }

    int enable = 1;
    result     = ::setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    VERBOSEF("::setsockopt(%d, SOL_SOCKET, SO_REUSEADDR, %p (%d), %d) = %d", listener_fd, &enable,
             enable, sizeof(enable), result);

    result = ::bind(listener_fd, reinterpret_cast<struct sockaddr*>(&addr_in), sizeof(addr_in));
    VERBOSEF("::bind(%d, %p, %d) = %d", listener_fd, &addr_in, sizeof(addr_in), result);
    if (result < 0) {
        ERRORF("bind failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        ::close(listener_fd);
        return false;
    }

    if (mPort == 0) {
        struct sockaddr_in bound_addr{};
        socklen_t          bound_len = sizeof(bound_addr);
        if (::getsockname(listener_fd, reinterpret_cast<struct sockaddr*>(&bound_addr),
                          &bound_len) == 0) {
            mPort = ntohs(bound_addr.sin_port);
            INFOF("bound to port %u", mPort);
        }
    }

    mListenerFd = listener_fd;
    return true;
}

std::string UdpInetListenerTask::event_name() const NOEXCEPT {
    return "udp-listener:" + mAddress + ":" + std::to_string(mPort);
}

UdpUnixListenerTask::UdpUnixListenerTask(std::string path) NOEXCEPT : mPath{std::move(path)} {
    VSCOPE_FUNCTIONF("\"%s\"", mPath.c_str());
}

bool UdpUnixListenerTask::create_socket() NOEXCEPT {
    struct sockaddr_un addr_un{};
    addr_un.sun_family = AF_UNIX;

    if (mPath.size() + 1 >= sizeof(addr_un.sun_path)) {
        ERRORF("path too long for unix socket: \"%s\"", mPath.c_str());
        return false;
    }

    memset(addr_un.sun_path, 0, sizeof(addr_un.sun_path));
    memcpy(addr_un.sun_path, mPath.c_str(), mPath.size());

    auto listener_fd = ::socket(AF_UNIX, SOCK_DGRAM, 0);
    VERBOSEF("::socket(AF_UNIX, SOCK_DGRAM, 0) = %d", listener_fd);
    if (listener_fd < 0) {
        ERRORF("socket failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        return false;
    }

    auto addr_len = static_cast<socklen_t>(sizeof(sa_family_t) + mPath.size() + 1);
    auto result   = ::bind(listener_fd, reinterpret_cast<struct sockaddr*>(&addr_un), addr_len);
    VERBOSEF("::bind(%d, %p, %d) = %d", listener_fd, &addr_un, addr_len, result);
    if (result < 0) {
        ERRORF("bind failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        ::close(listener_fd);
        return false;
    }

    mListenerFd = listener_fd;
    return true;
}

std::string UdpUnixListenerTask::event_name() const NOEXCEPT {
    return "udp-listener:" + mPath;
}

//
//
//

TcpConnectTask::TcpConnectTask(std::string host, uint16_t port, bool should_reconnect) NOEXCEPT
    : mState(StateUnscheduled),
      mScheduler{nullptr},
      mIsScheduled{false},
      mEvent{ScheduledEvent::invalid()},
      mPath{},
      mHost{std::move(host)},
      mPort{port},
      mReconnectTimeout{std::chrono::seconds{10}} {
    VSCOPE_FUNCTION();
    mConnected       = false;
    mShouldReconnect = should_reconnect;

    char name[256];
    snprintf(name, sizeof(name), "tcp:%s:%u", mHost.c_str(), mPort);
    mEventName = name;

    mReconnectTimeout.callback = [this]() {
        defer([this](Scheduler& sched) {
            mReconnectTimeout.cancel();
            if (!schedule(sched)) {
                ERRORF("failed to schedule reconnect timeout");
            }
        });
    };
}

TcpConnectTask::TcpConnectTask(std::string path, bool should_reconnect) NOEXCEPT
    : mState(StateUnscheduled),
      mScheduler{nullptr},
      mIsScheduled{false},
      mEvent{ScheduledEvent::invalid()},
      mPath{std::move(path)},
      mHost{},
      mPort{0},
      mReconnectTimeout{std::chrono::seconds{10}} {
    VSCOPE_FUNCTION();
    mConnected       = false;
    mShouldReconnect = should_reconnect;

    mEventName = "tcp:" + mPath;

    mReconnectTimeout.callback = [this]() {
        defer([this](Scheduler& sched) {
            mReconnectTimeout.cancel();
            if (!schedule(sched)) {
                ERRORF("failed to schedule reconnect timeout");
            }
        });
    };
}

TcpConnectTask::~TcpConnectTask() NOEXCEPT {
    VSCOPE_FUNCTION();

    disconnect();

    if (is_scheduled()) {
        cancel();
    }

    if (mFd >= 0) {
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
    }
}

void TcpConnectTask::set_reconnect_delay(std::chrono::milliseconds delay) NOEXCEPT {
    mReconnectTimeout.set_duration(delay);
}

char const* TcpConnectTask::state_to_string(State state) const NOEXCEPT {
    switch (state) {
    case StateUnscheduled: return "unscheduled";
    case StateConnecting: return "connecting";
    case StateConnected: return "connected";
    case StateDisconnected: return "disconnected";
    case StateError: return "error";
    }
    CORE_UNREACHABLE();
}

bool TcpConnectTask::connect() NOEXCEPT {
    VSCOPE_FUNCTIONF(") (state=%s", state_to_string(mState));
    if (mState != StateUnscheduled && mState != StateDisconnected) {
        WARNF("unexpected state: %s", state_to_string(mState));
        return false;
    }

    if (mHost.size() > 0) {
        // DNS lookup
        struct addrinfo* dns_result{};
        struct addrinfo  hints{};
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        char port[16];
        snprintf(port, sizeof(port), "%u", mPort);

        auto result = ::getaddrinfo(mHost.c_str(), port, &hints, &dns_result);
        VERBOSEF("::getaddrinfo(\"%s\", nullptr, %p, %p) = %d", mHost.c_str(), &hints, &dns_result,
                 result);
        if (result != 0) {
            ERRORF("getaddrinfo failed: %s", gai_strerror(result));
            mState = StateError;
            return false;
        }

        // get the first address
        mAddress       = {};
        mAddressLength = 0;
        for (auto addr = dns_result; addr != nullptr; addr = addr->ai_next) {
            char const* family = "AF_???";
            switch (addr->ai_family) {
            case AF_INET: family = "AF_INET"; break;
            case AF_INET6: family = "AF_INET6"; break;
            }
            char const* socket_type = "SOCK_???";
            switch (addr->ai_socktype) {
            case SOCK_STREAM: socket_type = "SOCK_STREAM"; break;
            case SOCK_DGRAM: socket_type = "SOCK_DGRAM"; break;
            }
            char const* protocol = "IPPROTO_???";
            switch (addr->ai_protocol) {
            case IPPROTO_TCP: protocol = "IPPROTO_TCP"; break;
            case IPPROTO_UDP: protocol = "IPPROTO_UDP"; break;
            }
            char buffer[512];
            if (addr->ai_family == AF_INET) {
                auto addr_in = reinterpret_cast<struct sockaddr_in*>(addr->ai_addr);
                ::inet_ntop(addr->ai_family, &addr_in->sin_addr, buffer, sizeof(buffer));
            } else if (addr->ai_family == AF_INET6) {
                auto addr_in6 = reinterpret_cast<struct sockaddr_in6*>(addr->ai_addr);
                ::inet_ntop(addr->ai_family, &addr_in6->sin6_addr, buffer, sizeof(buffer));
            } else {
                buffer[0] = '\0';
            }
            VERBOSEF("resolved address: %s %s %s %s", family, socket_type, protocol, buffer);

            if (mAddressLength == 0) {
                if (addr->ai_family == AF_INET || addr->ai_family == AF_INET6) {
                    mAddressLength = addr->ai_addrlen;
                    memcpy(&mAddress, addr->ai_addr, mAddressLength);
                }
            }
        }

        ::freeaddrinfo(dns_result);
        VERBOSEF("::freeaddrinfo(%p)", dns_result);

        if (mAddressLength == 0) {
            ERRORF("failed to resolve address");
            mState = StateError;
            return false;
        }
    } else if (mPath.size() > 0) {
        // create a socket address for a unix socket
        mAddress.ss_family = AF_UNIX;

        auto unix_addr = reinterpret_cast<struct sockaddr_un*>(&mAddress);
        if (mPath.size() + 1 >= sizeof(unix_addr->sun_path)) {
            ERRORF("path too long for unix socket: \"%s\"", mPath.c_str());
            mState = StateError;
            return false;
        }

        memset(unix_addr->sun_path, 0, sizeof(unix_addr->sun_path));
        memcpy(unix_addr->sun_path, mPath.c_str(), mPath.size());
        unix_addr->sun_path[mPath.size()] = '\0';
        mAddressLength = static_cast<socklen_t>(sizeof(sa_family_t) + mPath.size() + 1);
        VERBOSEF("unix socket path: %s", unix_addr->sun_path);
    } else {
        ERRORF("no host or path specified");
        mState = StateError;
        return false;
    }

    mFd = ::socket(mAddress.ss_family, SOCK_STREAM, 0);
    VERBOSEF("::socket(%d, SOCK_STREAM, 0) = %d", mAddress.ss_family, mFd);
    if (mFd < 0) {
        ERRORF("socket failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        mState = StateError;
        return false;
    }

    auto fcntl_flags = ::fcntl(mFd, F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", mFd, fcntl_flags);
    auto fcntl_reslut = ::fcntl(mFd, F_SETFL, fcntl_flags | O_NONBLOCK);
    VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", mFd, fcntl_flags | O_NONBLOCK, fcntl_reslut);

    auto result = ::connect(mFd, reinterpret_cast<struct sockaddr*>(&mAddress), mAddressLength);
    auto saved_errno = errno;
    VERBOSEF("::connect(%d, %p, %d) = %d", mFd, &mAddress, mAddressLength, result);
    if (result < 0) {
        if (saved_errno == EINPROGRESS) {
            // connection is in progress
            VERBOSEF("connection in progress");
            mState = StateConnecting;
        } else {
            if (mHost.size() > 0) {
                WARNF("connect failed: %s:%u, " ERRNO_FMT, mHost.c_str(), mPort,
                      ERRNO_ARGS(saved_errno));
            } else if (mPath.size() > 0) {
                WARNF("connect failed: \"%s\", " ERRNO_FMT, mPath.c_str(), ERRNO_ARGS(saved_errno));
            } else {
                WARNF("connect failed: " ERRNO_FMT, ERRNO_ARGS(saved_errno));
            }
            mState = StateError;
            return false;
        }
    } else {
        // connection is already established
        VERBOSEF("connection established");
        mState = StateConnected;

        if (on_connected) {
            on_connected(*this);
        }
    }

    return true;
}

void TcpConnectTask::disconnect() NOEXCEPT {
    VSCOPE_FUNCTIONF(") (state=%s", state_to_string(mState));
    if (mState != StateConnecting && mState != StateConnected) {
        WARNF("unexpected state: %s", state_to_string(mState));
        return;
    }

    cancel();

    if (mFd >= 0) {
        auto result = ::shutdown(mFd, SHUT_RDWR);
        VERBOSEF("::shutdown(%d, SHUT_RDWR) = %d", mFd, result);
        result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
    }

    if (mState == StateConnected) {
        if (on_disconnected) {
            on_disconnected(*this);
        }
    }

    mState = StateDisconnected;
}

void TcpConnectTask::event(EventInterest triggered) NOEXCEPT {
    VERBOSEF("event (state=%s) fd=%d triggered: %s%s%s%s", state_to_string(mState), mFd,
             (triggered & EventInterest::Read) ? "read " : "",
             (triggered & EventInterest::Write) ? "write " : "",
             (triggered & EventInterest::Error) ? "error " : "",
             (triggered & EventInterest::Hangup) ? "hangup " : "");
    TRACE_INDENT_SCOPE();

    if (triggered & (EventInterest::Error | EventInterest::Hangup)) {
        this->error();
    } else {
        if (triggered & EventInterest::Read) {
            this->read();
        }

        if (triggered & EventInterest::Write) {
            this->write();
        }
    }
}

void TcpConnectTask::read() NOEXCEPT {
    VSCOPE_FUNCTIONF(") (state=%s", state_to_string(mState));
    if (on_read) {
        on_read(*this);
    }
}

void TcpConnectTask::write() NOEXCEPT {
    VSCOPE_FUNCTIONF(") (state=%s", state_to_string(mState));
    if (mState == StateConnecting) {
        mState = StateConnected;
        if (on_connected) {
            on_connected(*this);
        }

        EventInterest interests = EventInterest::Error | EventInterest::Hangup;
        if (on_read) interests = interests | EventInterest::Read;
        mScheduler->update_interests(mEvent, interests);
        return;
    }

    if (on_write) {
        on_write(*this);
    }
}

void TcpConnectTask::error() NOEXCEPT {
    VSCOPE_FUNCTIONF(") (state=%s", state_to_string(mState));

    // get socket error
    int       error      = 0;
    socklen_t error_size = sizeof(error);
    auto      result     = ::getsockopt(mFd, SOL_SOCKET, SO_ERROR, &error, &error_size);
    VERBOSEF("::getsockopt(%d, SOL_SOCKET, SO_ERROR, %p, %p) = %d", mFd, &error, &error_size,
             result);
    if (result >= 0) {
        VERBOSEF("socket error: %s", strerror(error));
    }

    if (mState == StateConnecting) {
        if (mHost.size() > 0) {
            WARNF("connection failed: %s:%u, %d %s", mHost.c_str(), mPort, error, strerror(error));
        } else if (mPath.size() > 0) {
            WARNF("connection failed: \"%s\", %d %s", mPath.c_str(), error, strerror(error));
        } else {
            WARNF("connection failed: %d %s", error, strerror(error));
        }
        mScheduler->defer([this](scheduler::Scheduler&) {
            disconnect();
        });
    } else if (mState == StateConnected) {
        if (mHost.size() > 0) {
            WARNF("connection lost: %s:%u", mHost.c_str(), mPort);
        } else if (mPath.size() > 0) {
            WARNF("connection lost: \"%s\"", mPath.c_str());
        } else {
            WARNF("connection lost");
        }
        mScheduler->defer([this](scheduler::Scheduler&) {
            disconnect();
        });
    } else {
        WARNF("unexpected state: %s", state_to_string(mState));
        mScheduler->defer([this](scheduler::Scheduler&) {
            disconnect();
        });
    }

    if (mShouldReconnect && mScheduler) {
        if (!mReconnectTimeout.is_scheduled()) {
            VERBOSEF("schedule reconnect");
            defer([this](Scheduler&) {
                mReconnectTimeout.schedule();
            });
        }
    }
}

bool TcpConnectTask::schedule(Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p) (state=%s", &scheduler, state_to_string(mState));
    if (mIsScheduled) {
        WARNF("already scheduled");
        return false;
    } else if (!connect()) {
        if (mShouldReconnect) {
            if (!mReconnectTimeout.is_scheduled()) {
                VERBOSEF("schedule reconnect");
                mReconnectTimeout.schedule();
            }
            return true;
        } else {
            return false;
        }
    }

    if (mState != StateConnecting && mState != StateConnected) {
        WARNF("unexpected state: %s", state_to_string(mState));
        mState = StateError;
        return false;
    } else if (mFd < 0) {
        ERRORF("invalid file descriptor");
        mState = StateError;
        return false;
    }

    EventInterest interests = EventInterest::Error | EventInterest::Hangup;
    if (on_read) interests = interests | EventInterest::Read;
    if (on_write) interests = interests | EventInterest::Read;
    if (mState == StateConnecting) {
        interests = interests | EventInterest::Write;
    }

    mEvent = scheduler.register_fd(
        mFd, interests,
        [this](EventInterest triggered) {
            this->event(triggered);
        },
        mEventName);

    if (mEvent.valid()) {
        mConnected   = false;
        mScheduler   = &scheduler;
        mIsScheduled = true;
        return true;
    } else {
        mState = StateError;
        return false;
    }
}

bool TcpConnectTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTIONF(") (state=%s", state_to_string(mState));
    if (!mIsScheduled) {
        WARNF("not scheduled");
        return false;
    }

    mScheduler->unregister(mEvent);
    mIsScheduled = false;
    mEvent       = ScheduledEvent::invalid();
    return true;
}

void TcpConnectTask::update_interests(EventInterest interests) NOEXCEPT {
    VSCOPE_FUNCTIONF("%u) (state=%s", static_cast<uint32_t>(interests), state_to_string(mState));
    if (!mScheduler) {
        WARNF("not scheduled");
        return;
    }

    mScheduler->update_interests(mEvent, interests);
}

}  // namespace scheduler
