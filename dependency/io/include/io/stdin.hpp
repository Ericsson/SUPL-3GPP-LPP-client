#pragma once
#include <io/input.hpp>

#include <memory>

namespace scheduler {
class FileDescriptorTask;
}  // namespace scheduler

namespace io {
/// An input that reads from stdin.
class StdinInput : public Input {
public:
    EXPLICIT StdinInput() NOEXCEPT;
    ~StdinInput() NOEXCEPT override;

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::unique_ptr<scheduler::FileDescriptorTask> mFdTask;
    uint8_t mBuffer[4096];
};
}  // namespace io
