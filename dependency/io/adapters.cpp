#include <io/adapters.hpp>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(io, adapters);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(io, adapters)

namespace io {

StreamInputAdapter::StreamInputAdapter(std::shared_ptr<Stream> stream) NOEXCEPT
    : mStream(std::move(stream)) {
    VSCOPE_FUNCTIONF("stream=%s", mStream->id().c_str());
}

StreamInputAdapter::~StreamInputAdapter() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mReadHandle && mStream) {
        mStream->remove_on_read(mReadHandle);
    }
}

bool StreamInputAdapter::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p, stream=%s", &scheduler, mStream->id().c_str());

    mReadHandle = mStream->on_read([this](Stream&, uint8_t* data, size_t len) {
        TRACEF("received %zu bytes", len);
        if (callback) callback(*this, data, len);
    });

    mStream->on_complete = [this]() {
        VERBOSEF("stream completed");
        if (on_complete) on_complete();
    };

    if (mStream->state() == Stream::State::Initial) {
        return mStream->schedule(scheduler);
    }
    VERBOSEF("stream already scheduled");
    return true;
}

bool StreamInputAdapter::do_cancel(scheduler::Scheduler&) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mReadHandle) {
        mStream->remove_on_read(mReadHandle);
        mReadHandle = 0;
    }
    return true;
}

StreamOutputAdapter::StreamOutputAdapter(std::shared_ptr<Stream> stream) NOEXCEPT
    : mStream(std::move(stream)) {
    VSCOPE_FUNCTIONF("stream=%s", mStream->id().c_str());
}

StreamOutputAdapter::~StreamOutputAdapter() NOEXCEPT = default;

char const* StreamOutputAdapter::name() const NOEXCEPT {
    return "stream";
}

void StreamOutputAdapter::write(uint8_t const* buffer, size_t length) NOEXCEPT {
    TRACEF("%p, %zu", buffer, length);
    mStream->write(buffer, length);
}

bool StreamOutputAdapter::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p, stream=%s", &scheduler, mStream->id().c_str());
    if (mStream->state() == Stream::State::Initial) {
        return mStream->schedule(scheduler);
    }
    VERBOSEF("stream already scheduled");
    return true;
}

bool StreamOutputAdapter::do_cancel(scheduler::Scheduler&) NOEXCEPT {
    VSCOPE_FUNCTION();
    return true;
}

}  // namespace io
