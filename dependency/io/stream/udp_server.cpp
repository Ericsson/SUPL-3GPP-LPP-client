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

UdpServerStream::UdpServerStream(std::string                                       id,
                                 std::unique_ptr<scheduler::UdpSocketListenerTask> listener,
                                 ReadBufferConfig read_config) NOEXCEPT
    : Stream(std::move(id), read_config),
      mListenerTask(std::move(listener)) {
    VSCOPE_FUNCTIONF("\"%s\"", mId.c_str());
}

UdpServerStream::~UdpServerStream() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool UdpServerStream::schedule(scheduler::Scheduler& scheduler) {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    mScheduler = &scheduler;

    mListenerTask->on_read = [this](scheduler::UdpSocketListenerTask& task) {
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

    mListenerTask->on_error = [this](scheduler::UdpSocketListenerTask&) {
        ERRORF("listener error: " ERRNO_FMT, ERRNO_ARGS(errno));
        set_error(errno, "listener error");
    };

    mListenerTask->schedule(scheduler);
    if (!mListenerTask->is_scheduled()) {
        ERRORF("failed to schedule UDP listener");
        set_error(errno, "failed to schedule UDP listener");
        return false;
    }

    if (!schedule_read_timeout(scheduler)) {
        mListenerTask->cancel();
        return false;
    }

    mState = State::Connected;
    return true;
}

bool UdpServerStream::cancel() {
    VSCOPE_FUNCTION();
    cancel_read_timeout();
    if (mListenerTask) {
        mListenerTask->cancel();
    }
    return true;
}

uint16_t UdpServerStream::port() const NOEXCEPT {
    return mListenerTask ? mListenerTask->port() : 0;
}

void UdpServerStream::write(uint8_t const* data, size_t length) NOEXCEPT {
    TRACEF("%p, %zu", data, length);

    if (!mListenerTask || mLastSenderLen == 0) {
        WARNF("no destination address");
        return;
    }

    auto result = ::sendto(mListenerTask->fd(), data, length, 0,
                           reinterpret_cast<sockaddr*>(&mLastSender), mLastSenderLen);
    VERBOSEF("::sendto(%d, %p, %zu, 0, %p, %d) = %zd", mListenerTask->fd(), data, length,
             &mLastSender, mLastSenderLen, result);
    if (result < 0) {
        WARNF("sendto failed: " ERRNO_FMT, ERRNO_ARGS(errno));
    }
}

}  // namespace io
