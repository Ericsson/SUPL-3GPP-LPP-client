#include <client-io/tbin_input.hpp>
#include <loglet/loglet.hpp>

#include <chrono>

LOGLET_MODULE(tbin_input);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(tbin_input)

static int64_t now_us() {
    using namespace std::chrono;
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}

TbinInput::TbinInput(std::vector<std::string> paths, InputFormat format_mask,
                     bool replay_realtime) NOEXCEPT : mFormatMask(format_mask),
                                                      mRealtimeMode(replay_realtime),
                                                      mTask(std::chrono::milliseconds(0)) {
    mReaders.resize(paths.size());
    mPending.resize(paths.size());

    for (size_t i = 0; i < paths.size(); i++) {
        if (!mReaders[i].open(paths[i])) {
            ERRORF("tbin: failed to open %s", paths[i].c_str());
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

InputFormat TbinInput::detect_format(uint8_t const* data, size_t len, InputFormat mask) NOEXCEPT {
    // If only one format registered, use it directly
    auto bits = mask & (INPUT_FORMAT_UBX | INPUT_FORMAT_RTCM | INPUT_FORMAT_LPP_UPER |
                        INPUT_FORMAT_NMEA | INPUT_FORMAT_RAW);
    if (bits && (bits & (bits - 1)) == 0) return bits;  // single bit set

    // Auto-detect from payload
    if (len >= 2 && data[0] == 0xB5 && data[1] == 0x62 && (mask & INPUT_FORMAT_UBX))
        return INPUT_FORMAT_UBX;
    if (len >= 1 && data[0] == 0xD3 && (mask & INPUT_FORMAT_RTCM)) return INPUT_FORMAT_RTCM;
    if (mask & INPUT_FORMAT_LPP_UPER) return INPUT_FORMAT_LPP_UPER;
    if (mask & INPUT_FORMAT_RAW) return INPUT_FORMAT_RAW;
    return INPUT_FORMAT_NONE;
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
        if (format_callback) {
            auto fmt = detect_format(msg.data.data(), msg.data.size(), mFormatMask);
            format_callback(*this, fmt, msg.data.data(), msg.data.size());
        } else if (callback) {
            callback(*this, msg.data.data(), msg.data.size());
        }
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
