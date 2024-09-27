#pragma once
#include <core/core.hpp>

#include <functional>

namespace scheduler {
class Scheduler;
}

namespace io {
class Input {
public:
    EXPLICIT Input() NOEXCEPT;
    virtual ~Input() NOEXCEPT;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) NOEXCEPT;
    NODISCARD bool cancel() NOEXCEPT;

    std::function<void(Input&, uint8_t*, size_t)> callback;

protected:
    NODISCARD virtual bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT = 0;
    NODISCARD virtual bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT   = 0;

private:
    scheduler::Scheduler* mScheduler;
};
}  // namespace io
