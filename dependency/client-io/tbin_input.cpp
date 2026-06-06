#include <client-io/tbin_input.hpp>
#include <loglet/loglet.hpp>

#include <chrono>

LOGLET_MODULE(tbin_input);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(tbin_input)

static int64_t now_us() {
    using namespace std::chrono;
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}

TbinInput::TbinInput(std::vector<Source> sources, bool replay_realtime) NOEXCEPT
    : mRealtimeMode(replay_realtime),
      mTask(std::chrono::milliseconds(0)) {
    mReaders.resize(sources.size());
    mPending.resize(sources.size());
    mFormats.resize(sources.size());

    for (size_t i = 0; i < sources.size(); i++) {
        mFormats[i] = sources[i].format;
        if (!mReaders[i].open(sources[i].path)) {
            ERRORF("tbin: failed to open %s", sources[i].path.c_str());
            continue;
        }
        if (mReaders[i].next(mPending[i])) {
            mHeap.push({mPending[i].timestamp_us, static_cast<uint32_t>(i)});
        }
    }
}

TbinInput::~TbinInput() NOEXCEPT = default;

bool TbinInput::do_schedule(scheduler::Scheduler&) NOEXCEPT {
    mTask.callback = [this] {
        deliver_next();
    };
    mTask.schedule();
    return true;
}

bool TbinInput::do_cancel(scheduler::Scheduler&) NOEXCEPT {
    mTask.cancel();
    return true;
}

void TbinInput::deliver_next() NOEXCEPT {
    if (mHeap.empty()) {
        if (on_complete) on_complete();
        return;
    }

    auto top = mHeap.top();

    if (mRealtimeMode) {
        if (!mStarted) {
            mFirstTimestampUs = top.timestamp_us;
            mStartWallUs      = now_us();
            mStarted          = true;
        }
        int64_t delay_us = (top.timestamp_us - mFirstTimestampUs) - (now_us() - mStartWallUs);
        if (delay_us > 1000) {
            mTask.set_duration(std::chrono::microseconds(delay_us));
            mTask.restart();
            return;
        }
    }

    mHeap.pop();
    auto& msg = mPending[top.reader_index];

    if (!msg.data.empty()) {
        InputFormat fmt = mFormats[top.reader_index];
        if (format_callback)
            format_callback(*this, fmt, msg.data.data(), msg.data.size());
        else if (callback)
            callback(*this, msg.data.data(), msg.data.size());
    }

    if (mReaders[top.reader_index].next(mPending[top.reader_index])) {
        mHeap.push({mPending[top.reader_index].timestamp_us, top.reader_index});
    }

    if (!mHeap.empty()) {
        if (mRealtimeMode) {
            int64_t delay_us =
                (mHeap.top().timestamp_us - mFirstTimestampUs) - (now_us() - mStartWallUs);
            mTask.set_duration(delay_us > 0 ? std::chrono::microseconds(delay_us) :
                                              std::chrono::milliseconds(0));
        }
        mTask.restart();
    } else {
        if (on_complete) on_complete();
    }
}
