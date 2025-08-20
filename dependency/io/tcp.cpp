#include "tcp.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <functional>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <scheduler/scheduler.hpp>
#include <scheduler/socket.hpp>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(io, tcp);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(io, tcp)

namespace io {
TcpServerInput::TcpServerInput(std::string listen, uint16_t port) NOEXCEPT
    : mPath{},
      mListen(std::move(listen)),
      mPort(port) {
    VSCOPE_FUNCTIONF("\"%s\", %u", mListen.c_str(), mPort);
}

TcpServerInput::TcpServerInput(std::string path) NOEXCEPT : mPath(std::move(path)),
                                                            mListen{},
                                                            mPort(0) {
    VSCOPE_FUNCTIONF("\"%s\"", mPath.c_str());
}

TcpServerInput::~TcpServerInput() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool TcpServerInput::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    if (!mListen.empty())
        mListenerTask.reset(new scheduler::TcpListenerTask(mListen, mPort));
    else if (!mPath.empty())
        mListenerTask.reset(new scheduler::TcpListenerTask(mPath));
    else {
        ERRORF("no listen address or path specified");
        return false;
    }

    ASSERT(mListenerTask, "failed to create listener task");
    mListenerTask->on_accept = [this, &scheduler](scheduler::TcpListenerTask&, int data_fd,
                                                  struct sockaddr_storage*, socklen_t) {
        auto it = mClientTasks.begin();
        while (it != mClientTasks.end()) {
            if (!(*it)->is_scheduled()) {
                VERBOSEF("remove client task: %p", it->get());
                it = mClientTasks.erase(it);
            } else {
                ++it;
            }
        }

        auto client = std::unique_ptr<scheduler::SocketTask>(new scheduler::SocketTask(data_fd));
        client->on_read = [this](scheduler::SocketTask& task) {
            auto result = ::read(task.fd(), mBuffer, sizeof(mBuffer));
            VERBOSEF("::read(%d, %p, %zu) = %d", task.fd(), mBuffer, sizeof(mBuffer), result);
            if (result < 0) {
                ERRORF("failed to read from socket: " ERRNO_FMT, ERRNO_ARGS(errno));
                task.cancel();
                return;
            }

            if (callback) {
                callback(*this, mBuffer, static_cast<size_t>(result));
            }
        };
        client->on_error = [](scheduler::SocketTask& task) {
            // NOTE: I am not sure what to do here.
            task.cancel();
        };

        if (!client->schedule(scheduler)) {
            auto result = ::close(data_fd);
            VERBOSEF("::close(%d) = %d", data_fd, result);
        } else {
            VERBOSEF("add client task: %p", client.get());
            mClientTasks.push_back(std::move(client));
        }
    };
    mListenerTask->on_error = [this](scheduler::TcpListenerTask&) {
        // NOTE: I am not sure what to do here.
        cancel();
    };

    if (!mListenerTask->schedule(scheduler)) {
        mListenerTask.reset();
        return false;
    }

    return true;
}

bool TcpServerInput::do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    if (mListenerTask) {
        auto result = mListenerTask->cancel();
        mListenerTask.reset();
        return result;
    }

    return true;
}

//
//
//

TcpClientInput::TcpClientInput(std::string host, uint16_t port, bool reconnect) NOEXCEPT
    : mPath{},
      mHost(std::move(host)),
      mPort(port),
      mReconnect(reconnect) {
    VSCOPE_FUNCTIONF("\"%s\", %u", mHost.c_str(), mPort);
}

TcpClientInput::TcpClientInput(std::string path, bool reconnect) NOEXCEPT : mPath(std::move(path)),
                                                                            mHost{},
                                                                            mPort(0),
                                                                            mReconnect(reconnect) {
    VSCOPE_FUNCTIONF("\"%s\"", mPath.c_str());
}

TcpClientInput::~TcpClientInput() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool TcpClientInput::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    if (!mHost.empty())
        mConnectTask.reset(new scheduler::TcpConnectTask(mHost, mPort, mReconnect));
    else if (!mPath.empty())
        mConnectTask.reset(new scheduler::TcpConnectTask(mPath, mReconnect));
    else {
        ERRORF("no host or path specified");
        return false;
    }

    ASSERT(mConnectTask, "failed to create connect task");
    mConnectTask->on_read = [this](scheduler::TcpConnectTask& task) {
        auto result = ::read(task.fd(), mBuffer, sizeof(mBuffer));
        VERBOSEF("::read(%d, %p, %zu) = %d", task.fd(), mBuffer, sizeof(mBuffer), result);
        if (result < 0) {
            ERRORF("failed to read from socket: " ERRNO_FMT, ERRNO_ARGS(errno));
            mConnectTask->cancel();
            return;
        }

        if (callback) {
            callback(*this, mBuffer, static_cast<size_t>(result));
        }
    };

    if (!mConnectTask->schedule(scheduler)) {
        mConnectTask.reset();
        return false;
    }

    return true;
}

bool TcpClientInput::do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    if (mConnectTask) {
        mConnectTask->cancel();
        mConnectTask.reset();
    }

    return true;
}

//
//
//

TcpClientOutput::TcpClientOutput(std::string host, uint16_t port, bool reconnect) NOEXCEPT
    : mState(State::STATE_INITIAL),
      mHost(std::move(host)),
      mPort(port),
      mPath(),
      mReconnect(reconnect) {
    VSCOPE_FUNCTIONF("\"%s\", %u", mHost.c_str(), mPort);
    mFd = -1;
}

TcpClientOutput::TcpClientOutput(std::string path, bool reconnect) NOEXCEPT
    : mState(State::STATE_INITIAL),
      mHost(),
      mPort(0),
      mPath(std::move(path)),
      mReconnect(reconnect) {
    VSCOPE_FUNCTIONF("\"%s\"", mPath.c_str());
    mFd = -1;
}

TcpClientOutput::~TcpClientOutput() NOEXCEPT {
    VSCOPE_FUNCTION();
    disconnect();
}

void TcpClientOutput::write(uint8_t const* buffer, size_t length) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p, %zu) (state=%s", buffer, length, state_to_string(mState));

    if (mState == State::STATE_ERROR) {
        disconnect();
    }

    if (mReconnect && mState == State::STATE_DISCONNECTED) {
        if (mReconnectTime < std::chrono::steady_clock::now()) {
            mState = State::STATE_RECONNECT;
        }
    }

    if (mState == State::STATE_INITIAL || mState == State::STATE_RECONNECT) {
        connect();
    } else if (mState == State::STATE_CONNECTING) {
        connecting();
    }

    if (mState == State::STATE_CONNECTED) {
        auto result = ::send(mFd, buffer, length, MSG_NOSIGNAL);
        VERBOSEF("::send(%d, %p, %zu, MSG_NOSIGNAL) = %d", mFd, buffer, length, result);
        if (result < 0) {
            WARNF("failed to write to socket: " ERRNO_FMT, ERRNO_ARGS(errno));
            mState = STATE_ERROR;
        }
    }
}

bool TcpClientOutput::connect() NOEXCEPT {
    VSCOPE_FUNCTIONF(") (state=%s", state_to_string(mState));
    ASSERT(mState == State::STATE_INITIAL || mState == State::STATE_RECONNECT, "invalid state");

    mReconnectTime = std::chrono::steady_clock::now() + std::chrono::seconds(10);

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

    ASSERT(mAddressLength > 0, "invalid address length");
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
    }

    return true;
}

void TcpClientOutput::disconnect() NOEXCEPT {
    VSCOPE_FUNCTIONF(") (state=%s", state_to_string(mState));
    if (mFd >= 0) {
        auto result = ::shutdown(mFd, SHUT_RDWR);
        VERBOSEF("::shutdown(%d, SHUT_RDWR) = %d", mFd, result);
        result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
    }

    mState = STATE_DISCONNECTED;
}

bool TcpClientOutput::connecting() NOEXCEPT {
    VSCOPE_FUNCTIONF(") (state=%s", state_to_string(mState));

    // use poll to check for connection status or error
    struct pollfd poll_fd{};
    poll_fd.fd      = mFd;
    poll_fd.events  = POLLOUT | POLLERR | POLLHUP | POLLRDHUP;
    poll_fd.revents = 0;

    auto result = ::poll(&poll_fd, 1, 0);
    VERBOSEF("::poll(%p, 1, 0) = %d", &poll_fd, result);
    if (result < 0) {
        WARNF("poll failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        mState = STATE_ERROR;
        return false;
    }

    if (poll_fd.revents & POLLERR) {
        // get socket error
        int       error      = 0;
        socklen_t error_size = sizeof(error);

        result = ::getsockopt(mFd, SOL_SOCKET, SO_ERROR, &error, &error_size);
        VERBOSEF("::getsockopt(%d, SOL_SOCKET, SO_ERROR, %p, %p) = %d", mFd, &error, &error_size,
                 result);
        if (result >= 0) {
            VERBOSEF("socket error: %s", strerror(error));
        }

        if (mHost.size() > 0) {
            WARNF("connection failed: %s:%u, %s", mHost.c_str(), mPort, strerror(error));
        } else if (mPath.size() > 0) {
            WARNF("connection failed: \"%s\", %s", mPath.c_str(), strerror(error));
        } else {
            WARNF("connection failed: %s", strerror(error));
        }
        mState = STATE_ERROR;
        return false;
    }

    if (poll_fd.revents & POLLOUT) {
        VERBOSEF("connection established");
        mState = STATE_CONNECTED;
    }

    return true;
}

char const* TcpClientOutput::state_to_string(State state) const NOEXCEPT {
    switch (state) {
    case State::STATE_INITIAL: return "STATE_INITIAL";
    case State::STATE_CONNECTING: return "STATE_CONNECTING";
    case State::STATE_CONNECTED: return "STATE_CONNECTED";
    case State::STATE_DISCONNECTED: return "STATE_DISCONNECTED";
    case State::STATE_ERROR: return "STATE_ERROR";
    case State::STATE_RECONNECT: return "STATE_RECONNECT";
    default: CORE_UNREACHABLE();
    }
}

//
//
//

TcpServerOutput::TcpServerOutput(std::string listen, uint16_t port) NOEXCEPT
    : mPath(),
      mListen(std::move(listen)),
      mPort(port) {
    VSCOPE_FUNCTIONF("\"%s\", %u", mListen.c_str(), mPort);
}

TcpServerOutput::TcpServerOutput(std::string path) NOEXCEPT : mPath(std::move(path)),
                                                              mListen(),
                                                              mPort(0) {
    VSCOPE_FUNCTIONF("\"%s\"", mPath.c_str());
}

TcpServerOutput::~TcpServerOutput() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool TcpServerOutput::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    if (!mListen.empty())
        mListenerTask.reset(new scheduler::TcpListenerTask(mListen, mPort));
    else if (!mPath.empty())
        mListenerTask.reset(new scheduler::TcpListenerTask(mPath));
    else {
        ERRORF("no listen or path specified");
        return false;
    }

    ASSERT(mListenerTask, "failed to create listener task");
    mListenerTask->on_accept = [this, &scheduler](scheduler::TcpListenerTask&, int data_fd,
                                                  struct sockaddr_storage*, socklen_t) {
        // Cleanup removed clients
        for (auto& client : mRemoveClientTasks) {
            client.reset();
        }

        mRemoveClientTasks.clear();

        // Add new client
        auto client = std::unique_ptr<scheduler::SocketTask>(new scheduler::SocketTask(data_fd));
        client->on_error = [this](scheduler::SocketTask& task) {
            task.cancel();
            remove_client(task.fd());
        };

        if (!client->schedule(scheduler)) {
            auto result = ::close(data_fd);
            VERBOSEF("::close(%d) = %d", data_fd, result);
        } else {
            VERBOSEF("add client task: %p", client.get());
            mClientTasks.push_back(std::move(client));
        }
    };

    mListenerTask->on_error = [this](scheduler::TcpListenerTask&) {
        cancel();
    };

    if (!mListenerTask->schedule(scheduler)) {
        mListenerTask.reset();
        return false;
    }

    return true;
}

bool TcpServerOutput::do_cancel(scheduler::Scheduler&) NOEXCEPT {
    VSCOPE_FUNCTION();

    if (mListenerTask) {
        mListenerTask->cancel();
        mListenerTask.reset();
    }

    for (auto& client : mClientTasks) {
        client->cancel();
    }
    mClientTasks.clear();

    return true;
}

void TcpServerOutput::write(uint8_t const* buffer, size_t length) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p, %zu", buffer, length);

    for (auto& client : mClientTasks) {
        if (client->is_scheduled()) {
            auto result = ::send(client->fd(), buffer, length, MSG_NOSIGNAL);
            VERBOSEF("::send(%d, %p, %zu, MSG_NOSIGNAL) = %d", client->fd(), buffer, length,
                     result);
            if (result < 0) {
                WARNF("failed to write to socket: " ERRNO_FMT, ERRNO_ARGS(errno));
                client->cancel();
            }
        }
    }
}

void TcpServerOutput::remove_client(int fd) NOEXCEPT {
    VSCOPE_FUNCTIONF("%d", fd);

    auto client = std::find_if(mClientTasks.begin(), mClientTasks.end(),
                               [fd](std::unique_ptr<scheduler::SocketTask>& task) {
                                   return task->fd() == fd;
                               });

    if (client != mClientTasks.end()) {
        VERBOSEF("remove client task: %p", client->get());
        client->get()->cancel();

        mRemoveClientTasks.push_back(std::move(*client));
        mClientTasks.erase(client);
    }
}

}  // namespace io
