#pragma once
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>

#include <chrono>
#include <functional>

namespace scheduler {
class StreamTask {
public:
    EXPLICIT
    StreamTask(size_t block_size, std::chrono::steady_clock::duration duration,
               bool disable_pipe_buffer_optimization = false) NOEXCEPT;
    ~StreamTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    bool           cancel() NOEXCEPT;

    NODISCARD int fd() const NOEXCEPT { return mPipeFds[0]; }

    std::function<void(int fd, size_t block_size)> callback;

    void set_event_name(std::string const& name) { mPeriodicTask.set_event_name(name); }

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
                      std::chrono::steady_clock::duration duration,
                      bool disable_pipe_buffer_optimization = false) NOEXCEPT;
    ~ForwardStreamTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    bool           cancel() NOEXCEPT;

    NODISCARD int fd() const NOEXCEPT { return mStreamTask.fd(); }

    void set_event_name(std::string const& name) { mStreamTask.set_event_name(name); }

private:
    static constexpr size_t BUFFER_SIZE = 16 * 1024;

    void forward(int dest_fd, size_t block_size);

    StreamTask mStreamTask;
    int        mSourceFd;
    char       mBuffer[BUFFER_SIZE];
    size_t     mLeftOverCount;
};

}  // namespace scheduler
