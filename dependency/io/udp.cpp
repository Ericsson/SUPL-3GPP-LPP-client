#include "udp.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <scheduler/scheduler.hpp>
#include <scheduler/socket.hpp>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(io, udp);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(io, udp)

namespace io {
UdpServerInput::UdpServerInput(std::string listen, uint16_t port) NOEXCEPT
    : mPath{},
      mListen(std::move(listen)),
      mPort(port) {
    VSCOPE_FUNCTIONF("\"%s\", %u", mListen.c_str(), mPort);
}

UdpServerInput::UdpServerInput(std::string path) NOEXCEPT : mPath(std::move(path)),
                                                            mListen{},
                                                            mPort(0) {
    VSCOPE_FUNCTIONF("\"%s\"", mPath.c_str());
}

UdpServerInput::~UdpServerInput() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool UdpServerInput::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    if (!mListen.empty())
        mListenerTask.reset(new scheduler::UdpListenerTask(mListen, mPort));
    else if (!mPath.empty())
        mListenerTask.reset(new scheduler::UdpListenerTask(mPath));
    else {
        ERRORF("no listen address or path specified");
        return false;
    }

    ASSERT(mListenerTask, "failed to create listener task");
    mListenerTask->on_read = [this, &scheduler](scheduler::UdpListenerTask& task) {
        struct sockaddr_storage addr;
        socklen_t               addr_len = sizeof(addr);
        auto result = ::recvfrom(mListenerTask->fd(), mBuffer, sizeof(mBuffer), 0,
                                 reinterpret_cast<struct sockaddr*>(&addr), &addr_len);
        VERBOSEF("::recvfrom(%d, %p, %zu, 0, %p, %p) = %d", mListenerTask->fd(), mBuffer,
                 sizeof(mBuffer), &addr, &addr_len, result);
        if (result < 0) {
            ERRORF("failed to read from socket: " ERRNO_FMT, ERRNO_ARGS(errno));
            scheduler.defer([&task]() {
                task.cancel();
            });
            return;
        }

        if (callback) {
            callback(*this, mBuffer, static_cast<size_t>(result));
        }
    };
    mListenerTask->on_error = [this, &scheduler](scheduler::UdpListenerTask&) {
        scheduler.defer([this]() {
            cancel();
        });
    };

    if (!mListenerTask->schedule(scheduler)) {
        mListenerTask.reset();
        return false;
    }

    return true;
}

bool UdpServerInput::do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    if (mListenerTask) {
        mListenerTask->cancel();
        mListenerTask.reset();
    }

    return true;
}

//
//
//

UdpClientOutput::UdpClientOutput(std::string host, uint16_t port) NOEXCEPT : mHost(std::move(host)),
                                                                             mPort(port),
                                                                             mPath(""),
                                                                             mFd(-1) {
    VSCOPE_FUNCTIONF("\"%s\", %u", mHost.c_str(), mPort);
}

UdpClientOutput::UdpClientOutput(std::string path) NOEXCEPT : mHost(""),
                                                              mPort(0),
                                                              mPath(std::move(path)),
                                                              mFd(-1) {
    VSCOPE_FUNCTIONF("\"%s\"", mPath.c_str());
}

UdpClientOutput::~UdpClientOutput() NOEXCEPT {
    VSCOPE_FUNCTION();
    close();
}

void UdpClientOutput::write(uint8_t const* buffer, size_t length) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p, %zu", buffer, length);

    if (mFd == -1) {
        open();
    }

    if (mFd >= 0) {
        auto result = ::sendto(mFd, buffer, length, MSG_NOSIGNAL,
                               reinterpret_cast<struct sockaddr*>(&mAddress), mAddressLength);
        VERBOSEF("::sendto(%d, %p, %zu, MSG_NOSIGNAL, %p, %d) = %d", mFd, buffer, length, &mAddress,
                 mAddressLength, result);
        if (result < 0) {
            ERRORF("failed to write to socket: " ERRNO_FMT, ERRNO_ARGS(errno));
        }
    }
}

void UdpClientOutput::open() NOEXCEPT {
    VSCOPE_FUNCTION();

    if (mHost.size() > 0) {
        // DNS lookup
        struct addrinfo* dns_result{};
        struct addrinfo  hints{};
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;

        char port[16];
        snprintf(port, sizeof(port), "%u", mPort);

        auto result = ::getaddrinfo(mHost.c_str(), port, &hints, &dns_result);
        VERBOSEF("::getaddrinfo(\"%s\", nullptr, %p, %p) = %d", mHost.c_str(), &hints, &dns_result,
                 result);
        if (result != 0) {
            ERRORF("getaddrinfo failed: %s", gai_strerror(result));
            return;
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
            return;
        }
    } else if (mPath.size() > 0) {
        // create a socket address for a unix socket
        mAddress.ss_family = AF_UNIX;

        auto unix_addr = reinterpret_cast<struct sockaddr_un*>(&mAddress);
        if (mPath.size() + 1 >= sizeof(unix_addr->sun_path)) {
            ERRORF("path too long for unix socket: \"%s\"", mPath.c_str());
            return;
        }

        memset(unix_addr->sun_path, 0, sizeof(unix_addr->sun_path));
        memcpy(unix_addr->sun_path, mPath.c_str(), mPath.size());
        unix_addr->sun_path[mPath.size()] = '\0';
        mAddressLength = static_cast<socklen_t>(sizeof(sa_family_t) + mPath.size() + 1);
        VERBOSEF("unix socket path: %s", unix_addr->sun_path);
    } else {
        ERRORF("no host or path specified");
        return;
    }

    ASSERT(mAddressLength > 0, "address length is zero");
    mFd = ::socket(mAddress.ss_family, SOCK_DGRAM, 0);
    VERBOSEF("::socket(%d, SOCK_DGRAM, 0) = %d", mAddress.ss_family, mFd);
    if (mFd < 0) {
        ERRORF("socket failed: " ERRNO_FMT, ERRNO_ARGS(errno));
        return;
    }

    auto fcntl_flags = ::fcntl(mFd, F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", mFd, fcntl_flags);
    auto fcntl_reslut = ::fcntl(mFd, F_SETFL, fcntl_flags | O_NONBLOCK);
    VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", mFd, fcntl_flags | O_NONBLOCK, fcntl_reslut);
}

void UdpClientOutput::close() NOEXCEPT {
    VSCOPE_FUNCTION();

    if (mFd >= 0) {
        auto result = ::shutdown(mFd, SHUT_RDWR);
        VERBOSEF("::shutdown(%d, SHUT_RDWR) = %d", mFd, result);
        result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
    }
}

}  // namespace io
