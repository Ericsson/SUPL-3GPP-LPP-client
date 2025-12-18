#include <io/stream.hpp>
#include <scheduler/periodic.hpp>

#include <algorithm>
#include <cerrno>
#include <cstring>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(io, stream);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(io, stream)

namespace io {

Stream::Stream(std::string id, ReadBufferConfig read_config) NOEXCEPT : mId(std::move(id)),
                                                                        mReadConfig(read_config) {
    VSCOPE_FUNCTIONF("\"%s\"", mId.c_str());
}

Stream::~Stream() {
    VSCOPE_FUNCTION();
    cancel_read_timeout();
}

Stream::ReadCallbackHandle Stream::on_read(ReadCallback cb) NOEXCEPT {
    auto handle = mNextHandle++;
    mReadCallbacks.push_back({handle, std::move(cb)});
    VERBOSEF("registered read callback, handle=%zu", handle);
    return handle;
}

void Stream::remove_on_read(ReadCallbackHandle handle) NOEXCEPT {
    VERBOSEF("removing read callback, handle=%zu", handle);
    mReadCallbacks.erase(std::remove_if(mReadCallbacks.begin(), mReadCallbacks.end(),
                                        [handle](CallbackEntry& entry) {
                                            return entry.handle == handle;
                                        }),
                         mReadCallbacks.end());
}

void Stream::on_raw_read(uint8_t* data, size_t length) NOEXCEPT {
    TRACEF("%p, %zu", data, length);
    mReadBuffer.insert(mReadBuffer.end(), data, data + length);

    if (mReadBuffer.size() >= mReadConfig.min_bytes) {
        flush_read_buffer();
    }
}

void Stream::flush_read_buffer() NOEXCEPT {
    if (mReadBuffer.empty()) return;
    VERBOSEF("flushing read buffer, size=%zu", mReadBuffer.size());
    deliver_read(mReadBuffer.data(), mReadBuffer.size());
    mReadBuffer.clear();
}

void Stream::deliver_read(uint8_t* data, size_t length) NOEXCEPT {
    TRACEF("%p, %zu to %zu callbacks", data, length, mReadCallbacks.size());
    for (auto& entry : mReadCallbacks) {
        entry.callback(*this, data, length);
    }
}

void Stream::set_error(int error_code, std::string const& message) NOEXCEPT {
    ERRORF("stream error: %s (%d)", message.c_str(), error_code);
    mState = State::Error;
    if (on_error) on_error(*this, error_code, message);
    if (on_complete) on_complete();
}

void Stream::set_disconnected() NOEXCEPT {
    VERBOSEF("stream disconnected");
    mState = State::Disconnected;
    if (on_complete) on_complete();
}

bool Stream::schedule_read_timeout(scheduler::Scheduler& scheduler) NOEXCEPT {
    if (mReadConfig.timeout.count() <= 0) return true;

    VERBOSEF("scheduling read timeout: %lld ms",
             static_cast<long long>(mReadConfig.timeout.count()));
    mReadTimeoutTask.reset(new scheduler::PeriodicTask(mReadConfig.timeout));
    mReadTimeoutTask->callback = [this]() {
        flush_read_buffer();
    };
    return mReadTimeoutTask->schedule(scheduler);
}

void Stream::cancel_read_timeout() NOEXCEPT {
    if (mReadTimeoutTask) {
        VERBOSEF("cancelling read timeout");
        mReadTimeoutTask->cancel();
        mReadTimeoutTask.reset();
    }
}

}  // namespace io
