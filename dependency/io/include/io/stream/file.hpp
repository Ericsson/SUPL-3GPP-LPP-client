#pragma once
#include <io/stream.hpp>

#include <chrono>
#include <memory>
#include <string>

namespace scheduler {
class PeriodicTask;
class FileDescriptorTask;
}  // namespace scheduler

namespace io {

struct FileConfig {
    std::string path;
    bool        read     = true;
    bool        write    = false;
    bool        append   = false;
    bool        truncate = false;
    bool        create   = true;

    size_t                    bytes_per_tick = 0;
    std::chrono::milliseconds tick_interval  = {};
    ReadBufferConfig          read_config    = {};
};

class FileStream : public Stream {
public:
    EXPLICIT FileStream(std::string id, FileConfig config) NOEXCEPT;
    ~FileStream() NOEXCEPT override;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) override;
    bool           cancel() override;
    void           write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::string const& path() const NOEXCEPT { return mConfig.path; }

private:
    FileConfig mConfig;
    int        mFd = -1;

    std::unique_ptr<scheduler::PeriodicTask>       mReadTask;
    std::unique_ptr<scheduler::FileDescriptorTask> mFdTask;
    uint8_t                                        mReadBuf[4096 * 16];
};

}  // namespace io
