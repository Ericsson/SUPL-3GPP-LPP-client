#include "udp.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <scheduler/scheduler.hpp>
#include <scheduler/socket.hpp>

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "io"

namespace io {
UdpServerInput::UdpServerInput(std::string listen, uint16_t port) NOEXCEPT
    : mListen(std::move(listen)),
      mPort(port) {
    VSCOPE_FUNCTIONF("\"%s\", %u", mListen.c_str(), mPort);
}

UdpServerInput::~UdpServerInput() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool UdpServerInput::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    mListenerTask.reset(new scheduler::UdpListenerTask(mListen, mPort));
    mListenerTask->on_read = [this](scheduler::UdpListenerTask& task) {
        struct sockaddr_storage addr;
        socklen_t               addr_len = sizeof(addr);
        auto result = ::recvfrom(mListenerTask->fd(), mBuffer, sizeof(mBuffer), 0,
                                 reinterpret_cast<struct sockaddr*>(&addr), &addr_len);
        VERBOSEF("::recvfrom(%d, %p, %zu, 0, %p, %p) = %d", mListenerTask->fd(), mBuffer,
                 sizeof(mBuffer), &addr, &addr_len, result);
        if (result < 0) {
            ERRORF("failed to read from socket: " ERRNO_FMT, ERRNO_ARGS(errno));
            task.cancel();
            return;
        }

        if (callback) {
            callback(*this, mBuffer, static_cast<size_t>(result));
        }
    };
    mListenerTask->on_error = [this](scheduler::UdpListenerTask&) {
        // NOTE: I am not sure what to do here.
        cancel();
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
}  // namespace io
