#pragma once
#include <io/input.hpp>
#include <io/output.hpp>

#include <chrono>
#include <memory>
#include <string>

namespace scheduler {
class ForwardStreamTask;
class FileDescriptorTask;
}  // namespace scheduler

namespace io {
/// An input that reads from a file at a fixed rate.
class FileInput : public Input {
public:
    using Duration = std::chrono::steady_clock::duration;

    EXPLICIT FileInput(std::string path, size_t bytes_per_tick, Duration tick_interval) NOEXCEPT;
    ~FileInput() NOEXCEPT override;

    NODISCARD std::string const& path() const NOEXCEPT { return mPath; }
    NODISCARD size_t             bytes_per_tick() const NOEXCEPT { return mBytesPerTick; }
    NODISCARD Duration           tick_interval() const NOEXCEPT { return mTickInterval; }

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::string mPath;
    size_t      mBytesPerTick;
    Duration    mTickInterval;

    int mFileFd;
    int mForwardFd;

    std::unique_ptr<scheduler::ForwardStreamTask>  mStreamTask;
    std::unique_ptr<scheduler::FileDescriptorTask> mFdTask;

    uint8_t mBuffer[4096 * 16];
};

class FileOutput : public Output {
public:
    EXPLICIT FileOutput(std::string path, bool truncate, bool append, bool create) NOEXCEPT;
    ~FileOutput() NOEXCEPT override;

    void write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::string const& path() const NOEXCEPT { return mPath; }
    NODISCARD bool               truncate() const NOEXCEPT { return mTruncate; }
    NODISCARD bool               append() const NOEXCEPT { return mAppend; }
    NODISCARD bool               create() const NOEXCEPT { return mCreate; }

protected:
    void open();
    void close();

private:
    std::string mPath;
    bool        mTruncate;
    bool        mAppend;
    bool        mCreate;
    int         mFd;
};
}  // namespace io
