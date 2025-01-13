#include "stream.hpp"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "task"

namespace scheduler {
StreamTask::StreamTask(size_t                              block_size,
                       std::chrono::steady_clock::duration duration) NOEXCEPT
    : mPeriodicTask(duration),
      mBlockSize(block_size) {
    VSCOPE_FUNCTIONF("%zu, %ld ms", block_size,
                     std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    mPeriodicTask.callback = [this]() {
        VERBOSEF("stream task: event");
        if (this->callback) {
            TRACE_INDENT_SCOPE();
            this->callback(write_fd(), mBlockSize);
        }
    };

    mPipeFds[0] = -1;
    mPipeFds[1] = -1;
    auto result = ::pipe(mPipeFds);
    VERBOSEF("::pipe(%p) = %d", mPipeFds, result);
    if (result < 0) {
        ERRORF("failed to create pipe: " ERRNO_FMT, ERRNO_ARGS(errno));
        return;
    }

    VERBOSEF("pipe fds: %d, %d", mPipeFds[0], mPipeFds[1]);

    auto flags = ::fcntl(read_fd(), F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", read_fd(), flags);
    result = ::fcntl(read_fd(), F_SETFL, flags | O_NONBLOCK);
    VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", read_fd(), flags | O_NONBLOCK, result);
}

StreamTask::~StreamTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();

    if (mPipeFds[0] >= 0) {
        auto result = ::close(mPipeFds[0]);
        VERBOSEF("::close(%d) = %d", mPipeFds[0], result);
        mPipeFds[0] = -1;
    }

    if (mPipeFds[1] >= 0) {
        auto result = ::close(mPipeFds[1]);
        VERBOSEF("::close(%d) = %d", mPipeFds[1], result);
        mPipeFds[1] = -1;
    }
}

bool StreamTask::schedule(Scheduler& scheduler) NOEXCEPT {
    return mPeriodicTask.schedule(scheduler);
}

bool StreamTask::cancel() NOEXCEPT {
    return mPeriodicTask.cancel();
}

ForwardStreamTask::ForwardStreamTask(int source_fd, size_t block_size,
                                     std::chrono::steady_clock::duration duration)
    NOEXCEPT : mStreamTask(block_size, duration),
                         mSourceFd(source_fd) {
    VSCOPE_FUNCTIONF("%d, %zu, %ld ms", source_fd, block_size,
                     std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    mStreamTask.callback = [this](int dest_fd, size_t data_size) {
        forward(dest_fd, data_size);
    };
}

ForwardStreamTask::~ForwardStreamTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool ForwardStreamTask::schedule(Scheduler& scheduler) NOEXCEPT {
    return mStreamTask.schedule(scheduler);
}

bool ForwardStreamTask::cancel() NOEXCEPT {
    return mStreamTask.cancel();
}

void ForwardStreamTask::forward(int dest_fd, size_t block_size) {
    VSCOPE_FUNCTIONF("%d, %zu", dest_fd, block_size);
    char buffer[4096];

    size_t total = 0;
    while (total < block_size) {
        auto left = block_size - total;
        if (left > sizeof(buffer)) {
            left = sizeof(buffer);
        }

        auto count = ::read(mSourceFd, buffer, left);
        VERBOSEF("::read(%d, %p, %lu) = %ld", mSourceFd, buffer, left, count);
        if (count < 0) {
            ERRORF("failed to read from source: " ERRNO_FMT, ERRNO_ARGS(errno));
            return;
        }
        if (count == 0) {
            cancel();
            break;
        }

        auto written = ::write(dest_fd, buffer, static_cast<size_t>(count));
        VERBOSEF("write(%d, %p, %ld) = %ld", dest_fd, buffer, count, written);
        if (written < 0) {
            ERRORF("failed to write to destination: " ERRNO_FMT, ERRNO_ARGS(errno));
            return;
        }

        total += static_cast<size_t>(written);
    }
}
}  // namespace scheduler
