#include "stream.hpp"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(sched, stream);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(sched, stream)

namespace scheduler {
StreamTask::StreamTask(size_t block_size, std::chrono::steady_clock::duration duration,
                       bool disable_pipe_buffer_optimization) NOEXCEPT
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

    if (!disable_pipe_buffer_optimization) {
        size_t pipe_size = 64 * 1024 * 1024;
        while (pipe_size >= 1024 * 1024) {
            result = ::fcntl(write_fd(), F_SETPIPE_SZ, pipe_size);
            if (result >= 0) {
                VERBOSEF("pipe buffer size set to %zu bytes", pipe_size);
                break;
            }
            pipe_size /= 2;
        }
        if (result < 0) {
            VERBOSEF("failed to set pipe buffer size, using default");
        }
    }

    auto flags = ::fcntl(read_fd(), F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", read_fd(), flags);
    result = ::fcntl(read_fd(), F_SETFL, flags | O_NONBLOCK);
    VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", read_fd(), flags | O_NONBLOCK, result);

    flags = ::fcntl(write_fd(), F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", write_fd(), flags);
    result = ::fcntl(write_fd(), F_SETFL, flags | O_NONBLOCK);
    VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", write_fd(), flags | O_NONBLOCK, result);
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
                                     std::chrono::steady_clock::duration duration,
                                     bool disable_pipe_buffer_optimization) NOEXCEPT
    : mStreamTask(block_size, duration, disable_pipe_buffer_optimization),
      mSourceFd(source_fd) {
    VSCOPE_FUNCTIONF("%d, %zu, %ld ms", source_fd, block_size,
                     std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    mLeftOverCount       = 0;
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

    if (mLeftOverCount > 0) {
        DEBUGF("writing left over %ld bytes", mLeftOverCount);
        auto offset  = sizeof(mBuffer) - mLeftOverCount;
        auto written = ::write(dest_fd, mBuffer + offset, sizeof(mBuffer) - offset);
        VERBOSEF("write(%d, %p, %ld) = %ld", dest_fd, mBuffer + offset, sizeof(mBuffer) - offset,
                 written);
        if (written < 0) {
            WARNF("failed to write to destination: " ERRNO_FMT, ERRNO_ARGS(errno));
            return;
        }
        if (static_cast<size_t>(written) != sizeof(mBuffer) - mLeftOverCount) {
            DEBUGF("failed to write to destination: only wrote %ld bytes", written);
            mLeftOverCount -= written;
            return;
        }
        mLeftOverCount = 0;
    }

    size_t total = 0;
    while (total < block_size) {
        auto left = block_size - total;
        
#ifdef HAVE_SPLICE
        auto spliced = ::splice(mSourceFd, nullptr, dest_fd, nullptr, left, SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
        VERBOSEF("::splice(%d, NULL, %d, NULL, %zu, SPLICE_F_MOVE|SPLICE_F_NONBLOCK) = %ld", 
                 mSourceFd, dest_fd, left, spliced);
        
        if (spliced > 0) {
            total += static_cast<size_t>(spliced);
            continue;
        }
        
        if (spliced == 0) {
            cancel();
            break;
        }
        
        if (errno != EINVAL && errno != ENOSYS) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            ERRORF("splice failed: " ERRNO_FMT, ERRNO_ARGS(errno));
            return;
        }
#endif
        
        if (left > sizeof(mBuffer)) {
            left = sizeof(mBuffer);
        }

        auto count = ::read(mSourceFd, mBuffer, left);
        VERBOSEF("::read(%d, %p, %lu) = %ld", mSourceFd, mBuffer, left, count);
        if (count < 0) {
            ERRORF("failed to read from source: " ERRNO_FMT, ERRNO_ARGS(errno));
            return;
        }
        if (count == 0) {
            cancel();
            break;
        }

        auto written = ::write(dest_fd, mBuffer, static_cast<size_t>(count));
        VERBOSEF("write(%d, %p, %ld) = %ld", dest_fd, mBuffer, count, written);
        if (written < 0) {
            WARNF("failed to write to destination: " ERRNO_FMT, ERRNO_ARGS(errno));
            mLeftOverCount = static_cast<size_t>(count);
            return;
        }

        if (written != count) {
            DEBUGF("failed to write all data to destination: written: %ld, count: %ld", written,
                   count);
            mLeftOverCount = static_cast<size_t>(count) - written;
            return;
        }

        total += static_cast<size_t>(written);
    }
}
}  // namespace scheduler
