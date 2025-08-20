#include "socket.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <functional>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(sched, socket);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(sched, socket)

namespace scheduler {
ListenerTask::ListenerTask(int listener_fd) NOEXCEPT : mScheduler{nullptr},
                                                       mEvent{},
                                                       mListenerFd{listener_fd} {
    VSCOPE_FUNCTION();
    mEvent.event = [this](struct epoll_event* event) {
        this->event(event);
    };
}

ListenerTask::~ListenerTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (is_scheduled()) {
        cancel();
    }

    if (mListenerFd >= 0) {
        auto result = ::close(mListenerFd);
        VERBOSEF("::close(%d) = %d", mListenerFd, result);
        mListenerFd = -1;
    }
}

void ListenerTask::event(struct epoll_event* event) NOEXCEPT {
    VERBOSEF("event(%p) fd=%d events: %s%s%s%s%s", event, mListenerFd,
             (event->events & EPOLLIN) ? "read " : "", (event->events & EPOLLOUT) ? "write " : "",
             (event->events & EPOLLERR) ? "error " : "", (event->events & EPOLLHUP) ? "hup " : "",
             (event->events & EPOLLRDHUP) ? "rdhup " : "");
    TRACE_INDENT_SCOPE();

    if ((event->events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) != 0) {
        if (on_error) {
            this->on_error(*this);
        }
    } else if ((event->events & EPOLLIN) != 0) {
        this->accept();
    }
}

void ListenerTask::accept() NOEXCEPT {
    VSCOPE_FUNCTION();
    struct sockaddr_storage addr_storage{};
    socklen_t               addr_len = sizeof(addr_storage);
    auto                    client_fd =
        ::accept(mListenerFd, reinterpret_cast<struct sockaddr*>(&addr_storage), &addr_len);
    VERBOSEF("::accept(%d, %p, %p) = %d", mListenerFd, &addr_storage, &addr_len, client_fd);
    if (client_fd >= 0) {
        auto flags = ::fcntl(client_fd, F_GETFL, 0);
        VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", client_fd, flags);
        auto result = ::fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
        VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", client_fd, flags | O_NONBLOCK, result);

        if (on_accept) {
            on_accept(*this, client_fd, &addr_storage, addr_len);
        }
    } else {
        WARNF("accept failed: " ERRNO_FMT, ERRNO_ARGS(errno));
    }
}

bool ListenerTask::schedule(Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    if (mScheduler) {
        WARNF("already scheduled");
        return false;
    } else if (mListenerFd < 0) {
        WARNF("invalid file descriptor");
        return false;
    }

    // we're using epoll so we need to set the listener fd to non-blocking
    auto fcntl_flags = ::fcntl(mListenerFd, F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", mListenerFd, fcntl_flags);
    auto fcntl_reslut = ::fcntl(mListenerFd, F_SETFL, fcntl_flags | O_NONBLOCK);
    VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", mListenerFd, fcntl_flags | O_NONBLOCK, fcntl_reslut);

    // start listening
    auto result = ::listen(mListenerFd, SOMAXCONN);
    VERBOSEF("::listen(%d, %d) = %d", mListenerFd, SOMAXCONN, result);
    if (result < 0) {
        ERRORF("listen failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        return false;
    }

    uint32_t events = EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    if (on_accept) events |= EPOLLIN;
    if (scheduler.add_epoll_fd(mListenerFd, events, &mEvent)) {
        mScheduler = &scheduler;
        return true;
    } else {
        return false;
    }
}

bool ListenerTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mScheduler) {
        WARNF("not scheduled");
        return false;
    }

    if (mScheduler->remove_epoll_fd(mListenerFd)) {
        mScheduler = nullptr;
        return true;
    } else {
        return false;
    }
}

//
//
//

TcpListenerTask::TcpListenerTask(std::string address, uint16_t port) NOEXCEPT
    : mPath{},
      mAddress{std::move(address)},
      mPort{port},
      mListenerTask{nullptr} {
    VSCOPE_FUNCTION();
}

TcpListenerTask::TcpListenerTask(std::string path) NOEXCEPT : mPath{std::move(path)},
                                                              mAddress{},
                                                              mPort{0},
                                                              mListenerTask{nullptr} {
    VSCOPE_FUNCTION();
}

TcpListenerTask::~TcpListenerTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (is_scheduled()) {
        cancel();
    }
}

bool TcpListenerTask::schedule(Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    if (mListenerTask) {
        WARNF("already scheduled");
        return false;
    }

    struct sockaddr* addr     = nullptr;
    socklen_t        addr_len = 0;

    struct sockaddr_in addr_in{};
    struct sockaddr_un addr_un{};
    if (!mAddress.empty()) {
        addr_in.sin_family = AF_INET;
        addr_in.sin_port   = htons(mPort);
        auto result        = ::inet_pton(AF_INET, mAddress.c_str(), &addr_in.sin_addr);
        VERBOSEF("::inet_pton(AF_INET, %s, %p) = %d", mAddress.c_str(), &addr_in.sin_addr, result);
        if (result <= 0) {
            ERRORF("inet_pton failed: " ERRNO_FMT, ERRNO_ARGS(errno));
            return false;
        }
        addr     = reinterpret_cast<struct sockaddr*>(&addr_in);
        addr_len = sizeof(addr_in);
    } else if (!mPath.empty()) {
        addr_un.sun_family = AF_UNIX;

        if (mPath.size() + 1 >= sizeof(addr_un.sun_path)) {
            ERRORF("path too long for unix socket: \"%s\"", mPath.c_str());
            return false;
        }

        memset(addr_un.sun_path, 0, sizeof(addr_un.sun_path));
        memcpy(addr_un.sun_path, mPath.c_str(), mPath.size());
        addr_un.sun_path[mPath.size()] = '\0';
        addr                           = reinterpret_cast<struct sockaddr*>(&addr_un);
        addr_len = static_cast<socklen_t>(sizeof(sa_family_t) + mPath.size() + 1);
    } else {
        ERRORF("no listen address or path specified");
        return false;
    }

    ASSERT(addr, "invalid address");
    ASSERT(addr_len > 0, "invalid address length");
    auto listener_fd = ::socket(addr->sa_family, SOCK_STREAM, 0);
    VERBOSEF("::socket(%d, SOCK_STREAM, 0) = %d", addr->sa_family, listener_fd);
    if (listener_fd < 0) {
        ERRORF("socket failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        return false;
    }

    int  enable = 1;
    auto result = ::setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    VERBOSEF("::setsockopt(%d, SOL_SOCKET, SO_REUSEADDR, %p (%d), %d) = %d", listener_fd, &enable,
             enable, sizeof(enable), result);
    if (result < 0) {
        WARNF("setsockopt failed: " ERRNO_FMT, ERRNO_ARGS(errno));
    }

    result = ::bind(listener_fd, addr, addr_len);
    VERBOSEF("::bind(%d, %p, %d) = %d", listener_fd, addr, addr_len, result);
    if (result < 0) {
        ERRORF("bind failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        ::close(listener_fd);
        return false;
    }

    mListenerTask.reset(new ListenerTask(listener_fd));
    mListenerTask->on_accept = [this](ListenerTask&, int client_fd,
                                      struct sockaddr_storage* client_addr,
                                      socklen_t                client_addr_len) {
        if (on_accept) {
            on_accept(*this, client_fd, client_addr, client_addr_len);
        }
    };
    mListenerTask->on_error = [this](ListenerTask&) {
        if (on_error) {
            on_error(*this);
        }
    };

    if (mListenerTask->schedule(scheduler)) {
        return true;
    } else {
        mListenerTask.reset();
        return false;
    }
}

bool TcpListenerTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mListenerTask) {
        WARNF("not scheduled");
        return false;
    }

    if (mListenerTask->cancel()) {
        mListenerTask.reset();
        return true;
    } else {
        return false;
    }
}

//
//
//

UdpListenerTask::UdpListenerTask(std::string address, uint16_t port) NOEXCEPT
    : mPath{},
      mAddress{std::move(address)},
      mPort{port},
      mScheduler{nullptr},
      mListenerFd{-1} {
    VSCOPE_FUNCTIONF("\"%s\", %u", mAddress.c_str(), mPort);

    mEvent.event = [this](struct epoll_event* event) {
        this->event(event);
    };
}

UdpListenerTask::UdpListenerTask(std::string path) NOEXCEPT : mPath{std::move(path)},
                                                              mAddress{},
                                                              mPort{0},
                                                              mScheduler{nullptr},
                                                              mListenerFd{-1} {
    VSCOPE_FUNCTIONF("\"%s\"", mPath.c_str());

    mEvent.event = [this](struct epoll_event* event) {
        this->event(event);
    };
}

UdpListenerTask::~UdpListenerTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (is_scheduled()) {
        cancel();
    }
}

void UdpListenerTask::event(struct epoll_event* event) NOEXCEPT {
    VERBOSEF("event(%p) fd=%d events: %s%s%s%s%s", event, mListenerFd,
             (event->events & EPOLLIN) ? "read " : "", (event->events & EPOLLOUT) ? "write " : "",
             (event->events & EPOLLERR) ? "error " : "", (event->events & EPOLLHUP) ? "hup " : "",
             (event->events & EPOLLRDHUP) ? "rdhup " : "");
    TRACE_INDENT_SCOPE();

    if ((event->events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) != 0) {
        if (on_error) {
            this->on_error(*this);
        }
    } else if ((event->events & EPOLLIN) != 0) {
        if (on_read) {
            this->on_read(*this);
        }
    }
}

bool UdpListenerTask::schedule(Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    if (mScheduler) {
        WARNF("already scheduled");
        return false;
    } else if (mListenerFd >= 0) {
        WARNF("already scheduled");
        return false;
    }

    struct sockaddr* addr     = nullptr;
    socklen_t        addr_len = 0;

    struct sockaddr_in addr_in{};
    struct sockaddr_un addr_un{};
    if (!mAddress.empty()) {
        addr_in.sin_family = AF_INET;
        addr_in.sin_port   = htons(mPort);
        auto result        = ::inet_pton(AF_INET, mAddress.c_str(), &addr_in.sin_addr);
        VERBOSEF("::inet_pton(AF_INET, %s, %p) = %d", mAddress.c_str(), &addr_in.sin_addr, result);
        if (result <= 0) {
            ERRORF("inet_pton failed: " ERRNO_FMT, ERRNO_ARGS(errno));
            return false;
        }
        addr     = reinterpret_cast<struct sockaddr*>(&addr_in);
        addr_len = sizeof(addr_in);
    } else if (!mPath.empty()) {
        addr_un.sun_family = AF_UNIX;

        if (mPath.size() + 1 >= sizeof(addr_un.sun_path)) {
            ERRORF("path too long for unix socket: \"%s\"", mPath.c_str());
            return false;
        }

        memset(addr_un.sun_path, 0, sizeof(addr_un.sun_path));
        memcpy(addr_un.sun_path, mPath.c_str(), mPath.size());
        addr_un.sun_path[mPath.size()] = '\0';
        addr                           = reinterpret_cast<struct sockaddr*>(&addr_un);
        addr_len = static_cast<socklen_t>(sizeof(sa_family_t) + mPath.size() + 1);
    } else {
        ERRORF("no listen address or path specified");
        return false;
    }

    ASSERT(addr, "invalid address");
    ASSERT(addr_len > 0, "invalid address length");
    auto listener_fd = ::socket(addr->sa_family, SOCK_DGRAM, 0);
    VERBOSEF("::socket(%d, SOCK_DGRAM, 0) = %d", addr->sa_family, listener_fd);
    if (listener_fd < 0) {
        ERRORF("socket failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        return false;
    }

    int  enable = 1;
    auto result = ::setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    VERBOSEF("::setsockopt(%d, SOL_SOCKET, SO_REUSEADDR, %p (%d), %d) = %d", listener_fd, &enable,
             enable, sizeof(enable), result);
    if (result < 0) {
        WARNF("setsockopt failed: " ERRNO_FMT, ERRNO_ARGS(errno));
    }

    result = ::bind(listener_fd, addr, addr_len);
    VERBOSEF("::bind(%d, %p, %d) = %d", listener_fd, addr, addr_len, result);
    if (result < 0) {
        ERRORF("bind failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        ::close(listener_fd);
        return false;
    }

    uint32_t events = EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    if (on_read) events |= EPOLLIN;
    if (scheduler.add_epoll_fd(listener_fd, events, &mEvent)) {
        mScheduler  = &scheduler;
        mListenerFd = listener_fd;
        return true;
    } else {
        result = ::close(listener_fd);
        VERBOSEF("::close(%d) = %d", listener_fd, result);
        return false;
    }
}

bool UdpListenerTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();

    if (!mScheduler) {
        WARNF("not scheduled");
        return false;
    }

    if (mScheduler->remove_epoll_fd(mListenerFd)) {
        mScheduler = nullptr;

        auto result = ::close(mListenerFd);
        VERBOSEF("::close(%d) = %d", mListenerFd, result);
        mListenerFd = -1;
        return true;
    } else {
        return false;
    }
}

//
//
//

SocketTask::SocketTask(int fd) NOEXCEPT : mScheduler(nullptr), mEvent{}, mFd(fd) {
    VSCOPE_FUNCTION();
    mEvent.event = [this](struct epoll_event* event) {
        this->event(event);
    };
}

SocketTask::~SocketTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (is_scheduled()) {
        cancel();
    }

    if (mFd >= 0) {
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
    }
}

void SocketTask::event(struct epoll_event* event) NOEXCEPT {
    VERBOSEF("event(%p) fd=%d events: %s%s%s%s%s", event, mFd,
             (event->events & EPOLLIN) ? "read " : "", (event->events & EPOLLOUT) ? "write " : "",
             (event->events & EPOLLERR) ? "error " : "", (event->events & EPOLLHUP) ? "hup " : "",
             (event->events & EPOLLRDHUP) ? "rdhup " : "");
    TRACE_INDENT_SCOPE();

    if ((event->events & EPOLLIN) != 0) {
        this->read();
    }

    if ((event->events & EPOLLOUT) != 0) {
        this->write();
    }

    if ((event->events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) != 0) {
        this->error();
    }
}

void SocketTask::read() NOEXCEPT {
    if (on_read) {
        on_read(*this);
    }
}

void SocketTask::write() NOEXCEPT {
    if (on_write) {
        on_write(*this);
    }
}

void SocketTask::error() NOEXCEPT {
    if (on_error) {
        on_error(*this);
    }
}

bool SocketTask::schedule(Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    if (mScheduler) {
        WARNF("already scheduled");
        return false;
    } else if (mFd < 0) {
        WARNF("invalid file descriptor");
        return false;
    }

    uint32_t events = EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    if (on_read) events |= EPOLLIN;
    if (on_write) events |= EPOLLOUT;
    if (scheduler.add_epoll_fd(mFd, events, &mEvent)) {
        mScheduler = &scheduler;
        return true;
    } else {
        return false;
    }
}

bool SocketTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mScheduler) {
        WARNF("not scheduled");
        return false;
    }

    if (mScheduler->remove_epoll_fd(mFd)) {
        mScheduler = nullptr;
        return true;
    } else {
        return false;
    }
}

//
//
//

TcpConnectTask::TcpConnectTask(std::string host, uint16_t port, bool should_reconnect) NOEXCEPT
    : mState(STATE_UNSCEDULED),
      mScheduler{nullptr},
      mIsScheduled{false},
      mEvent{},
      mPath{},
      mHost{std::move(host)},
      mPort{port},
      mReconnectTimeout{std::chrono::seconds{10}} {
    VSCOPE_FUNCTION();
    mConnected       = false;
    mShouldReconnect = should_reconnect;

    mEvent.event = [this](struct epoll_event* event) {
        this->event(event);
    };

    mReconnectTimeout.callback = [this]() {
        auto scheduler = &mReconnectTimeout.scheduler();
        mReconnectTimeout.cancel();
        schedule(*scheduler);
    };
}

TcpConnectTask::TcpConnectTask(std::string path, bool should_reconnect) NOEXCEPT
    : mState(STATE_UNSCEDULED),
      mScheduler{nullptr},
      mIsScheduled{false},
      mEvent{},
      mPath{std::move(path)},
      mHost{},
      mPort{0},
      mReconnectTimeout{std::chrono::seconds{10}} {
    VSCOPE_FUNCTION();
    mConnected       = false;
    mShouldReconnect = should_reconnect;

    mEvent.name  = "socket";
    mEvent.event = [this](struct epoll_event* event) {
        this->event(event);
    };

    mReconnectTimeout.callback = [this]() {
        auto scheduler = &mReconnectTimeout.scheduler();
        mReconnectTimeout.cancel();
        schedule(*scheduler);
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

char const* TcpConnectTask::state_to_string(State state) const NOEXCEPT {
    switch (state) {
    case STATE_UNSCEDULED: return "unscheduled";
    case STATE_CONNECTING: return "connecting";
    case STATE_CONNECTED: return "connected";
    case STATE_DISCONNECTED: return "disconnected";
    case STATE_ERROR: return "error";
    }
}

bool TcpConnectTask::connect() NOEXCEPT {
    VSCOPE_FUNCTIONF(") (state=%s", state_to_string(mState));
    if (mState != STATE_UNSCEDULED && mState != STATE_DISCONNECTED) {
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
            mState = STATE_ERROR;
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
            mState = STATE_ERROR;
            return false;
        }
    } else if (mPath.size() > 0) {
        // create a socket address for a unix socket
        mAddress.ss_family = AF_UNIX;

        auto unix_addr = reinterpret_cast<struct sockaddr_un*>(&mAddress);
        if (mPath.size() + 1 >= sizeof(unix_addr->sun_path)) {
            ERRORF("path too long for unix socket: \"%s\"", mPath.c_str());
            mState = STATE_ERROR;
            return false;
        }

        memset(unix_addr->sun_path, 0, sizeof(unix_addr->sun_path));
        memcpy(unix_addr->sun_path, mPath.c_str(), mPath.size());
        unix_addr->sun_path[mPath.size()] = '\0';
        mAddressLength = static_cast<socklen_t>(sizeof(sa_family_t) + mPath.size() + 1);
        VERBOSEF("unix socket path: %s", unix_addr->sun_path);
    } else {
        ERRORF("no host or path specified");
        mState = STATE_ERROR;
        return false;
    }

    mFd = ::socket(mAddress.ss_family, SOCK_STREAM, 0);
    VERBOSEF("::socket(%d, SOCK_STREAM, 0) = %d", mAddress.ss_family, mFd);
    if (mFd < 0) {
        ERRORF("socket failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        mState = STATE_ERROR;
        return false;
    }

    auto fcntl_flags = ::fcntl(mFd, F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", mFd, fcntl_flags);
    auto fcntl_reslut = ::fcntl(mFd, F_SETFL, fcntl_flags | O_NONBLOCK);
    VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", mFd, fcntl_flags | O_NONBLOCK, fcntl_reslut);

    auto result = ::connect(mFd, reinterpret_cast<struct sockaddr*>(&mAddress), mAddressLength);
    VERBOSEF("::connect(%d, %p, %d) = %d", mFd, &mAddress, mAddressLength, result);
    if (result < 0) {
        if (errno == EINPROGRESS) {
            // connection is in progress
            VERBOSEF("connection in progress");
            mState = STATE_CONNECTING;
        } else {
            if (mHost.size() > 0) {
                WARNF("connect failed: %s:%u, " ERRNO_FMT, mHost.c_str(), mPort, ERRNO_ARGS(errno));
            } else if (mPath.size() > 0) {
                WARNF("connect failed: \"%s\", " ERRNO_FMT, mPath.c_str(), ERRNO_ARGS(errno));
            } else {
                WARNF("connect failed: " ERRNO_FMT, ERRNO_ARGS(errno));
            }
            mState = STATE_ERROR;
            return false;
        }
    } else {
        // connection is already established
        VERBOSEF("connection established");
        mState = STATE_CONNECTED;

        if (on_connected) {
            on_connected(*this);
        }
    }

    return true;
}

void TcpConnectTask::disconnect() NOEXCEPT {
    VSCOPE_FUNCTIONF(") (state=%s", state_to_string(mState));
    if (mState != STATE_CONNECTING && mState != STATE_CONNECTED) {
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

    if (mState == STATE_CONNECTED) {
        if (on_disconnected) {
            on_disconnected(*this);
        }
    }

    mState = STATE_DISCONNECTED;
}

void TcpConnectTask::event(struct epoll_event* event) NOEXCEPT {
    VERBOSEF("event(%p) (state=%s) fd=%d events: %s%s%s%s%s", event, state_to_string(mState), mFd,
             (event->events & EPOLLIN) ? "read " : "", (event->events & EPOLLOUT) ? "write " : "",
             (event->events & EPOLLERR) ? "error " : "", (event->events & EPOLLHUP) ? "hup " : "",
             (event->events & EPOLLRDHUP) ? "rdhup " : "");
    TRACE_INDENT_SCOPE();

    if ((event->events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) != 0) {
        this->error();
    } else {
        if ((event->events & EPOLLIN) != 0) {
            this->read();
        }

        if ((event->events & EPOLLOUT) != 0) {
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
    if (mState == STATE_CONNECTING) {
        mState = STATE_CONNECTED;
        if (on_connected) {
            on_connected(*this);
        }

        if (!on_write) {
            // when we're connected and there's no write callback we don't need to
            // monitor for write events
            uint32_t events = EPOLLERR | EPOLLHUP | EPOLLRDHUP;
            if (on_read) events |= EPOLLIN;
            mScheduler->update_epoll_fd(mFd, events, &mEvent);
        }
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

    if (mState == STATE_CONNECTING) {
        if (mHost.size() > 0) {
            WARNF("connection failed: %s:%u, " ERRNO_FMT, mHost.c_str(), mPort, ERRNO_ARGS(errno));
        } else if (mPath.size() > 0) {
            WARNF("connection failed: \"%s\", " ERRNO_FMT, mPath.c_str(), ERRNO_ARGS(errno));
        } else {
            WARNF("connection failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        }
        disconnect();
    } else if (mState == STATE_CONNECTED) {
        if (mHost.size() > 0) {
            WARNF("connection lost: %s:%u", mHost.c_str(), mPort);
        } else if (mPath.size() > 0) {
            WARNF("connection lost: \"%s\"", mPath.c_str());
        } else {
            WARNF("connection lost");
        }
        disconnect();
    } else {
        WARNF("unexpected state: %s", state_to_string(mState));
        disconnect();
    }

    if (mShouldReconnect && mScheduler) {
        if (!mReconnectTimeout.is_scheduled()) {
            VERBOSEF("schedule reconnect");
            mReconnectTimeout.schedule(*mScheduler);
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
                mReconnectTimeout.schedule(scheduler);
            }
            return true;
        } else {
            return false;
        }
    }

    if (mState != STATE_CONNECTING && mState != STATE_CONNECTED) {
        WARNF("unexpected state: %s", state_to_string(mState));
        mState = STATE_ERROR;
        return false;
    } else if (mFd < 0) {
        ERRORF("invalid file descriptor");
        mState = STATE_ERROR;
        return false;
    }

    uint32_t events = EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    if (on_read) events |= EPOLLIN;
    if (on_write) events |= EPOLLIN;
    if (mState == STATE_CONNECTING) {
        events |= EPOLLOUT;
    }
    if (scheduler.add_epoll_fd(mFd, events, &mEvent)) {
        mConnected   = false;
        mScheduler   = &scheduler;
        mIsScheduled = true;
        return true;
    } else {
        mState = STATE_ERROR;
        return false;
    }
}

bool TcpConnectTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTIONF(") (state=%s", state_to_string(mState));
    if (!mIsScheduled) {
        WARNF("not scheduled");
        return false;
    }

    if (mScheduler->remove_epoll_fd(mFd)) {
        mIsScheduled = false;
        return true;
    } else {
        mState = STATE_ERROR;
        return false;
    }
}

}  // namespace scheduler
