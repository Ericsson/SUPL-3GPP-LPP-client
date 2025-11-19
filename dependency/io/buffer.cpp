#include "buffer.hpp"

#include <fcntl.h>
#include <loglet/loglet.hpp>
#include <scheduler/scheduler.hpp>
#include <unistd.h>

LOGLET_MODULE2(io, buffer);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(io, buffer)

namespace io {

BufferOutput::BufferOutput()  = default;
BufferOutput::~BufferOutput() = default;

void BufferOutput::write(uint8_t const* buffer, size_t length) NOEXCEPT {
    mBuffer.insert(mBuffer.end(), buffer, buffer + length);
    if (on_write) {
        on_write(buffer, length);
    }
}

BufferInput::BufferInput() {
    FUNCTION_SCOPE();
    if (pipe(mPipeFds) == -1) {
        ERRORF("failed to create pipe");
        mPipeFds[0] = -1;
        mPipeFds[1] = -1;
        return;
    }

    int flags = fcntl(mPipeFds[0], F_GETFL, 0);
    fcntl(mPipeFds[0], F_SETFL, flags | O_NONBLOCK);

    mEvent.name  = "buffer-input";
    mEvent.event = [this](struct epoll_event*) {
        on_readable();
    };
}

BufferInput::~BufferInput() {
    FUNCTION_SCOPE();
    if (mPipeFds[0] != -1) close(mPipeFds[0]);
    if (mPipeFds[1] != -1) close(mPipeFds[1]);
}

void BufferInput::push(uint8_t const* data, size_t length) {
    if (mPipeFds[1] != -1) {
        ::write(mPipeFds[1], data, length);
    }
}

void BufferInput::push(std::vector<uint8_t> const& data) {
    push(data.data(), data.size());
}

void BufferInput::on_readable() {
    uint8_t buffer[4096];
    ssize_t n = ::read(mPipeFds[0], buffer, sizeof(buffer));
    if (n > 0 && callback) {
        callback(static_cast<Input&>(*this), buffer, static_cast<size_t>(n));
    }
}

bool BufferInput::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    FUNCTION_SCOPE();
    if (mPipeFds[0] == -1) return false;
    return scheduler.add_epoll_fd(mPipeFds[0], EPOLLIN, &mEvent);
}

bool BufferInput::do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT {
    FUNCTION_SCOPE();
    if (mPipeFds[0] == -1) return true;
    return scheduler.remove_epoll_fd(mPipeFds[0]);
}

}  // namespace io
