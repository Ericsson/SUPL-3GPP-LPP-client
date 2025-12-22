#pragma once
#include <scheduler/scheduler.hpp>

#include <functional>
#include <string>

namespace scheduler {

class FileDescriptorTask {
public:
    EXPLICIT FileDescriptorTask() NOEXCEPT;
    ~FileDescriptorTask() NOEXCEPT;

    FileDescriptorTask(FileDescriptorTask&& other) NOEXCEPT;
    FileDescriptorTask& operator=(FileDescriptorTask&& other) NOEXCEPT;
    FileDescriptorTask(FileDescriptorTask const&)            = delete;
    FileDescriptorTask& operator=(FileDescriptorTask const&) = delete;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    NODISCARD bool schedule() NOEXCEPT { return schedule(current()); }
    bool           cancel() NOEXCEPT;
    NODISCARD bool is_scheduled() const NOEXCEPT { return mEvent.valid(); }

    void          set_fd(int fd) NOEXCEPT;
    NODISCARD int fd() const NOEXCEPT { return mFd; }

    void update() NOEXCEPT;
    void update_interests(EventInterest interests) NOEXCEPT;

    void set_name(std::string name) NOEXCEPT { mName = std::move(name); }
    void set_event_name(std::string name) NOEXCEPT { set_name(std::move(name)); }

    std::function<void(FileDescriptorTask&)> on_read;
    std::function<void(FileDescriptorTask&)> on_write;
    std::function<void(FileDescriptorTask&)> on_error;

private:
    ScheduledEvent mEvent;
    std::string    mName;
    int            mFd;
};

class OwnedFileDescriptorTask {
public:
    EXPLICIT OwnedFileDescriptorTask(int fd = -1) NOEXCEPT;
    ~OwnedFileDescriptorTask() NOEXCEPT;

    OwnedFileDescriptorTask(OwnedFileDescriptorTask&& other) NOEXCEPT;
    OwnedFileDescriptorTask& operator=(OwnedFileDescriptorTask&& other) NOEXCEPT;
    OwnedFileDescriptorTask(OwnedFileDescriptorTask const&)            = delete;
    OwnedFileDescriptorTask& operator=(OwnedFileDescriptorTask const&) = delete;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT { return mTask.schedule(scheduler); }
    NODISCARD bool schedule() NOEXCEPT { return schedule(current()); }
    bool           cancel() NOEXCEPT { return mTask.cancel(); }
    NODISCARD bool is_scheduled() const NOEXCEPT { return mTask.is_scheduled(); }

    void          set_fd(int fd) NOEXCEPT;
    NODISCARD int fd() const NOEXCEPT { return mTask.fd(); }
    int           release() NOEXCEPT;

    void update() NOEXCEPT { mTask.update(); }
    void update_interests(EventInterest interests) NOEXCEPT { mTask.update_interests(interests); }

    void set_name(std::string name) NOEXCEPT { mTask.set_name(std::move(name)); }
    void set_event_name(std::string name) NOEXCEPT { set_name(std::move(name)); }

    std::function<void(OwnedFileDescriptorTask&)> on_read;
    std::function<void(OwnedFileDescriptorTask&)> on_write;
    std::function<void(OwnedFileDescriptorTask&)> on_error;

private:
    FileDescriptorTask mTask;
};

}  // namespace scheduler
