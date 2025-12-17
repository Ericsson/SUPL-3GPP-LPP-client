#pragma once
#include <core/core.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace scheduler {
class Scheduler;
class PeriodicTask;
}  // namespace scheduler

namespace io {

struct ReadBufferConfig {
    size_t                    min_bytes = 1;
    std::chrono::milliseconds timeout   = {};
};

class Stream {
public:
    enum class State {
        Initial,
        Connecting,
        Connected,
        Disconnected,
        Error,
    };

    using ReadCallback       = std::function<void(Stream&, uint8_t*, size_t)>;
    using ReadCallbackHandle = size_t;

    EXPLICIT Stream(std::string id, ReadBufferConfig read_config = {}) NOEXCEPT;
    virtual ~Stream();

    NODISCARD virtual bool schedule(scheduler::Scheduler& scheduler)            = 0;
    virtual bool           cancel()                                             = 0;
    virtual void           write(uint8_t const* buffer, size_t length) NOEXCEPT = 0;

    NODISCARD virtual size_t pending_writes() const NOEXCEPT { return 0; }

    ReadCallbackHandle on_read(ReadCallback cb) NOEXCEPT;
    void               remove_on_read(ReadCallbackHandle handle) NOEXCEPT;

    std::function<void()>                                            on_complete;
    std::function<void(Stream&, int error_code, std::string const&)> on_error;

    NODISCARD std::string const& id() const NOEXCEPT { return mId; }
    NODISCARD State              state() const NOEXCEPT { return mState; }

protected:
    std::string           mId;
    State                 mState     = State::Initial;
    scheduler::Scheduler* mScheduler = nullptr;

    ReadBufferConfig                         mReadConfig;
    std::vector<uint8_t>                     mReadBuffer;
    std::unique_ptr<scheduler::PeriodicTask> mReadTimeoutTask;

    struct CallbackEntry {
        ReadCallbackHandle handle;
        ReadCallback       callback;
    };
    std::vector<CallbackEntry> mReadCallbacks;
    ReadCallbackHandle         mNextHandle = 1;

    void on_raw_read(uint8_t* data, size_t length) NOEXCEPT;
    void flush_read_buffer() NOEXCEPT;
    void deliver_read(uint8_t* data, size_t length) NOEXCEPT;

    void set_error(int error_code, std::string const& message) NOEXCEPT;
    void set_disconnected() NOEXCEPT;

    bool schedule_read_timeout(scheduler::Scheduler& scheduler) NOEXCEPT;
    void cancel_read_timeout() NOEXCEPT;
};

}  // namespace io
