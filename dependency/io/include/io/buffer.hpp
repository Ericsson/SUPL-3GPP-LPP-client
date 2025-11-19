#pragma once
#include <io/input.hpp>
#include <io/output.hpp>
#include <scheduler/scheduler.hpp>

#include <functional>
#include <vector>

namespace io {

class BufferOutput : public Output {
public:
    EXPLICIT BufferOutput();
    ~BufferOutput() override;

    NODISCARD const char* name() const NOEXCEPT override { return "buffer"; }

    void write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::vector<uint8_t> const& data() const NOEXCEPT { return mBuffer; }
    void                                  clear() NOEXCEPT { mBuffer.clear(); }

    std::function<void(uint8_t const*, size_t)> on_write;

private:
    std::vector<uint8_t> mBuffer;
};

class BufferInput : public Input {
public:
    EXPLICIT BufferInput();
    ~BufferInput() override;

    void push(uint8_t const* data, size_t length);
    void push(std::vector<uint8_t> const& data);

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    void on_readable();

    int                   mPipeFds[2];
    scheduler::EpollEvent mEvent;
};

}  // namespace io
