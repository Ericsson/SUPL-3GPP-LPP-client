#include <client-io/tbin_input.hpp>
#include <loglet/loglet.hpp>

#include <chrono>
#include <ctime>

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
    mShifts.resize(sources.size());

    DEBUGF("tbin: %zu source(s), realtime=%s", sources.size(), replay_realtime ? "true" : "false");
    for (size_t i = 0; i < sources.size(); i++) {
        mFormats[i] = sources[i].format;
        mShifts[i]  = sources[i].shift_us;
        DEBUGF("tbin: source[%zu] path=%s format=0x%llx shift=%llds", i, sources[i].path.c_str(),
               (unsigned long long)sources[i].format, (long long)(sources[i].shift_us / 1000000LL));
        if (!mReaders[i].open(sources[i].path)) {
            ERRORF("tbin: failed to open %s", sources[i].path.c_str());
            continue;
        }
        if (mReaders[i].next(mPending[i])) {
            DEBUGF("tbin: source[%zu] first ts=%lld", i, (long long)mPending[i].timestamp_us);
            mHeap.push({mPending[i].timestamp_us + mShifts[i], static_cast<uint32_t>(i)});
        } else {
            WARNF("tbin: source[%zu] empty or unreadable", i);
        }
    }
    DEBUGF("tbin: heap size=%zu after init", mHeap.size());
}

TbinInput::~TbinInput() NOEXCEPT = default;

bool TbinInput::do_schedule(scheduler::Scheduler&) NOEXCEPT {
    DEBUGF("tbin: scheduling, heap=%zu", mHeap.size());
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

next_message:
    auto top = mHeap.top();

    // Stop if we've passed the stop time
    if (mStopTimeUs > 0 && top.timestamp_us > mStopTimeUs) {
        if (on_complete) on_complete();
        return;
    }

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

    // Log progress every 60s of data time
    if (top.timestamp_us - mLastLogUs >= 60LL * 1000000LL) {
        auto t = static_cast<time_t>(top.timestamp_us / 1000000LL);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));
        fprintf(stderr, "[tbin] replay at %s\n", buf);
        mLastLogUs = top.timestamp_us;
    }

    if (!msg.data.empty()) {
        InputFormat fmt = mFormats[top.reader_index];
        if (format_callback)
            format_callback(*this, fmt, msg.data.data(), msg.data.size());
        else if (callback)
            callback(*this, msg.data.data(), msg.data.size());

        // In non-realtime mode drain all pending streamline queue events now,
        // so queues don't overflow before the epoll loop gets a chance to run.
        if (!mRealtimeMode) {
            while (scheduler::current().execute_once() == scheduler::ExecuteResult::ConditionMet) {
            }
        }
    }

    if (mReaders[top.reader_index].next(mPending[top.reader_index])) {
        mHeap.push({mPending[top.reader_index].timestamp_us + mShifts[top.reader_index],
                    top.reader_index});
    }

    if (!mHeap.empty()) {
        if (mRealtimeMode) {
            int64_t delay_us =
                (mHeap.top().timestamp_us - mFirstTimestampUs) - (now_us() - mStartWallUs);
            mTask.set_duration(delay_us > 0 ? std::chrono::microseconds(delay_us) :
                                              std::chrono::nanoseconds(1));
            mTask.restart();
        } else {
            // Non-realtime: continue looping without going back through epoll
            goto next_message;
        }
    } else {
        if (on_complete) on_complete();
    }
}
