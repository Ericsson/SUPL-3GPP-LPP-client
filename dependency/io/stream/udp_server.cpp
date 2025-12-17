#include <io/stream/udp_server.hpp>
#include <scheduler/socket.hpp>

#include <cerrno>
#include <cstring>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE3(io, stream, udp_server);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(io, stream, udp_server)

namespace io {

UdpServerStream::UdpServerStream(std::string id, UdpServerConfig config) NOEXCEPT
    : Stream(std::move(id), config.read_config),
      mConfig(std::move(config)) {
    VSCOPE_FUNCTIONF("\"%s\", listen=\"%s\", port=%u, path=\"%s\"", mId.c_str(),
                     mConfig.listen.c_str(), mConfig.port, mConfig.path.c_str());
}

UdpServerStream::~UdpServerStream() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool UdpServerStream::schedule(scheduler::Scheduler& scheduler) {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    mScheduler = &scheduler;

    if (!mConfig.path.empty()) {
        INFOF("listening on unix datagram socket: %s", mConfig.path.c_str());
        mListenerTask.reset(new scheduler::UdpListenerTask(mConfig.path));
    } else {
        INFOF("listening on %s:%u", mConfig.listen.c_str(), mConfig.port);
        mListenerTask.reset(new scheduler::UdpListenerTask(mConfig.listen, mConfig.port));
    }

    mListenerTask->on_read = [this](scheduler::UdpListenerTask& task) {
        mLastSenderLen = sizeof(mLastSender);
        auto result    = ::recvfrom(task.fd(), mReadBuf, sizeof(mReadBuf), 0,
                                    reinterpret_cast<sockaddr*>(&mLastSender), &mLastSenderLen);
        VERBOSEF("::recvfrom(%d, %p, %zu, 0, %p, %p) = %zd", task.fd(), mReadBuf, sizeof(mReadBuf),
                 &mLastSender, &mLastSenderLen, result);
        if (result > 0) {
            on_raw_read(mReadBuf, result);
        } else if (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            ERRORF("failed to read from socket: " ERRNO_FMT, ERRNO_ARGS(errno));
            set_error(errno, strerror(errno));
        }
    };

    mListenerTask->on_error = [this](scheduler::UdpListenerTask&) {
        ERRORF("listener error: " ERRNO_FMT, ERRNO_ARGS(errno));
        set_error(errno, "listener error");
    };

    if (!mListenerTask->schedule(scheduler)) {
        ERRORF("failed to schedule UDP listener");
        set_error(errno, "failed to schedule UDP listener");
        return false;
    }

    if (!schedule_read_timeout(scheduler)) {
        mListenerTask->cancel();
        return false;
    }

    mState = State::Connected;
    DEBUGF("udp server listening");
    return true;
}

bool UdpServerStream::cancel() {
    VSCOPE_FUNCTION();
    cancel_read_timeout();
    if (mListenerTask) {
        mListenerTask->cancel();
        mListenerTask.reset();
    }
    return true;
}

uint16_t UdpServerStream::actual_port() const NOEXCEPT {
    return mListenerTask ? mListenerTask->port() : 0;
}

void UdpServerStream::write(uint8_t const* data, size_t length) NOEXCEPT {
    if (mLastSenderLen == 0) {
        WARNF("cannot write: no sender address");
        return;
    }
    auto result = ::sendto(mListenerTask->fd(), data, length, MSG_NOSIGNAL,
                           reinterpret_cast<sockaddr*>(&mLastSender), mLastSenderLen);
    VERBOSEF("::sendto(%d, %p, %zu, MSG_NOSIGNAL, %p, %u) = %zd", mListenerTask->fd(), data, length,
             &mLastSender, mLastSenderLen, result);
    if (result < 0) {
        WARNF("failed to write to socket: " ERRNO_FMT, ERRNO_ARGS(errno));
    }
}

}  // namespace io
