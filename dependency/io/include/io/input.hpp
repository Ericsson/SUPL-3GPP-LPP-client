#pragma once
#include <core/core.hpp>

#include <functional>
#include <string>

namespace scheduler {
class Scheduler;
}

namespace io {
class Input {
public:
    EXPLICIT Input() NOEXCEPT;
    virtual ~Input() NOEXCEPT;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) NOEXCEPT;
    bool cancel() NOEXCEPT;

    std::function<void(Input&, uint8_t*, size_t)> callback;

    NODISCARD std::string const& event_name() const NOEXCEPT { return mEventName; }

    void set_event_name(std::string const& name) NOEXCEPT {
        mEventName = name;
        on_event_name_changed();
    }

protected:
    NODISCARD virtual bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT = 0;
    NODISCARD virtual bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT   = 0;

    virtual void on_event_name_changed() NOEXCEPT {}

protected:
    std::string mEventName;

private:
    scheduler::Scheduler* mScheduler;
};
}  // namespace io
