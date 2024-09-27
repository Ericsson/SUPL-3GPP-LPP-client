#pragma once
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>

#include <chrono>
#include <functional>

namespace scheduler {
class StreamTask {
public:
    EXPLICIT
    StreamTask(size_t block_size, std::chrono::steady_clock::duration duration) NOEXCEPT;
    ~StreamTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    NODISCARD bool cancel() NOEXCEPT;

    NODISCARD int fd() const NOEXCEPT { return mPipeFds[0]; }

    std::function<void(int fd, size_t block_size)> callback;

private:
    NODISCARD int read_fd() const NOEXCEPT { return mPipeFds[0]; }
    NODISCARD int write_fd() const NOEXCEPT { return mPipeFds[1]; }

    PeriodicTask mPeriodicTask;
    size_t       mBlockSize;
    int          mPipeFds[2];
};

class ForwardStreamTask {
public:
    EXPLICIT
    ForwardStreamTask(int source_fd, size_t block_size,
                      std::chrono::steady_clock::duration duration) NOEXCEPT;
    ~ForwardStreamTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    NODISCARD bool cancel() NOEXCEPT;

    NODISCARD int fd() const NOEXCEPT { return mStreamTask.fd(); }

private:
    void forward(int dest_fd, size_t block_size);

    StreamTask mStreamTask;
    int        mSourceFd;
};

}  // namespace scheduler
