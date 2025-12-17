#pragma once
#include <io/input.hpp>
#include <io/output.hpp>
#include <io/stream.hpp>

#include <memory>

namespace io {

class StreamInputAdapter : public Input {
public:
    EXPLICIT StreamInputAdapter(std::shared_ptr<Stream> stream) NOEXCEPT;
    ~StreamInputAdapter() NOEXCEPT override;

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::shared_ptr<Stream>    mStream;
    Stream::ReadCallbackHandle mReadHandle = 0;
};

class StreamOutputAdapter : public Output {
public:
    EXPLICIT StreamOutputAdapter(std::shared_ptr<Stream> stream) NOEXCEPT;
    ~StreamOutputAdapter() NOEXCEPT override;

    NODISCARD char const* name() const NOEXCEPT override;
    void                  write(uint8_t const* buffer, size_t length) NOEXCEPT override;

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::shared_ptr<Stream> mStream;
};

}  // namespace io
