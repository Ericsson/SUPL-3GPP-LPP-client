#pragma once
#include <core/core.hpp>

namespace scheduler {
class Scheduler;
}

namespace io {
class Output {
public:
    EXPLICIT Output() NOEXCEPT;
    virtual ~Output() NOEXCEPT;

    virtual char const* name() const NOEXCEPT = 0;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) NOEXCEPT;
    bool cancel() NOEXCEPT;

    virtual void write(uint8_t const* buffer, size_t length) NOEXCEPT = 0;

protected:
    NODISCARD virtual bool do_schedule(scheduler::Scheduler&) NOEXCEPT { return true; }
    NODISCARD virtual bool do_cancel(scheduler::Scheduler&) NOEXCEPT { return true; }

private:
    scheduler::Scheduler* mScheduler;
};
}  // namespace io
