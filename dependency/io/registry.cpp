#include <io/registry.hpp>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(io, registry);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(io, registry)

namespace io {

void StreamRegistry::add(std::string const& id, std::shared_ptr<Stream> stream) NOEXCEPT {
    VSCOPE_FUNCTIONF("\"%s\"", id.c_str());
    mStreams[id] = std::move(stream);
    DEBUGF("registered stream: %s", id.c_str());
}

std::shared_ptr<Stream> StreamRegistry::get(std::string const& id) const NOEXCEPT {
    TRACEF("\"%s\"", id.c_str());
    auto it = mStreams.find(id);
    if (it == mStreams.end()) {
        VERBOSEF("stream not found: %s", id.c_str());
        return nullptr;
    }
    return it->second;
}

bool StreamRegistry::has(std::string const& id) const NOEXCEPT {
    return mStreams.find(id) != mStreams.end();
}

bool StreamRegistry::schedule_all(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p, count=%zu", &scheduler, mStreams.size());
    for (auto& [id, stream] : mStreams) {
        if (stream->state() == Stream::State::Initial) {
            DEBUGF("scheduling stream: %s", id.c_str());
            if (!stream->schedule(scheduler)) {
                ERRORF("failed to schedule stream: %s", id.c_str());
                return false;
            }
        }
    }
    return true;
}

void StreamRegistry::cancel_all() NOEXCEPT {
    VSCOPE_FUNCTION();
    for (auto& [id, stream] : mStreams) {
        DEBUGF("cancelling stream: %s", id.c_str());
        stream->cancel();
    }
}

}  // namespace io
