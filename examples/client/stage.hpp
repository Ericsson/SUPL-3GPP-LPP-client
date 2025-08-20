#pragma once

#include <io/output.hpp>
#include <scheduler/scheduler.hpp>

#include "input_format.hpp"
#include "output_format.hpp"

class OutputStage {
public:
    EXPLICIT OutputStage() NOEXCEPT;
    virtual ~OutputStage() NOEXCEPT;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) NOEXCEPT;
    NODISCARD bool cancel() NOEXCEPT;

    virtual void write(OutputFormat format, uint8_t const* buffer, size_t length) NOEXCEPT = 0;

protected:
    NODISCARD virtual bool do_schedule(scheduler::Scheduler&) NOEXCEPT { return true; }
    NODISCARD virtual bool do_cancel(scheduler::Scheduler&) NOEXCEPT { return true; }

private:
    scheduler::Scheduler* mScheduler;
};

class InputStage {
public:
    EXPLICIT InputStage() NOEXCEPT;
    virtual ~InputStage() NOEXCEPT;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) NOEXCEPT;
    NODISCARD bool cancel() NOEXCEPT;

    std::function<void(InputFormat, uint8_t*, size_t)> callback;

protected:
    NODISCARD virtual bool do_schedule(scheduler::Scheduler&) NOEXCEPT { return true; }
    NODISCARD virtual bool do_cancel(scheduler::Scheduler&) NOEXCEPT { return true; }

private:
    scheduler::Scheduler* mScheduler;
};
