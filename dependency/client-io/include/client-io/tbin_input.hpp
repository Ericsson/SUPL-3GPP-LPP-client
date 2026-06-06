#pragma once
#include <client-io/input_format.hpp>
#include <format/tbin/reader.hpp>
#include <io/input.hpp>
#include <scheduler/timeout.hpp>

#include <memory>
#include <queue>
#include <string>
#include <vector>

// TbinInput: reads one or more .tbin files, delivers payload bytes in timestamp order.
//
// Multiple paths are merged by min-heap — messages are delivered in timestamp order
// across all files. This handles the case where ubx.tbin and ssr.tbin were captured
// simultaneously and need to be replayed interleaved.
//
// format_mask: which formats to auto-detect (used when multiple formats present).
//   - If only one format bit set: all messages treated as that format.
//   - If multiple bits set: detect from payload bytes (UBX/RTCM/LPP-UPER).
//   The detected format is passed to the format_callback instead of the raw callback.
//
// replay_realtime=true: deliver at recorded wall-clock rate.
class TbinInput : public io::Input {
public:
    // Extended callback that also delivers the detected format of each message.
    // Use this instead of io::Input::callback when multiple formats are possible.
    std::function<void(TbinInput&, InputFormat, uint8_t*, size_t)> format_callback;

    std::function<void()> on_complete;

    explicit TbinInput(std::vector<std::string> paths, InputFormat format_mask = INPUT_FORMAT_RAW,
                       bool replay_realtime = false) NOEXCEPT;
    ~TbinInput() NOEXCEPT override;

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    void deliver_next() NOEXCEPT;

    static InputFormat detect_format(uint8_t const* data, size_t len, InputFormat mask) NOEXCEPT;

    struct Entry {
        int64_t  timestamp_us;
        uint32_t reader_index;
        bool     operator>(Entry const& o) const { return timestamp_us > o.timestamp_us; }
    };

    std::vector<format::tbin::Reader>                                   mReaders;
    std::vector<format::tbin::Message>                                  mPending;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> mHeap;

    InputFormat mFormatMask;
    bool        mRealtimeMode;
    int64_t     mFirstTimestampUs = 0;
    int64_t     mStartWallUs      = 0;
    bool        mStarted          = false;

    scheduler::RepeatableTimeoutTask mTask;
};
