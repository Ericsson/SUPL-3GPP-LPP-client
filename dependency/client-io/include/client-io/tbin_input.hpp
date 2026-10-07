#pragma once
#include <client-io/input_format.hpp>
#include <format/tbin/reader.hpp>
#include <io/input.hpp>
#include <scheduler/timeout.hpp>

#include <functional>
#include <queue>
#include <string>
#include <vector>

// TbinInput: reads one or more .tbin files, delivers payload bytes in strict timestamp order.
//
// Each file has its own fixed format (from --input tbin:path=X,format=Y).
// Multiple files are merged via a min-heap — messages are interleaved by timestamp.
// format_callback delivers each message with the format of the file it came from.
class TbinInput : public io::Input {
public:
    // The constructors are explicit rather than relying on aggregate initialization: a default
    // member initializer makes a struct a non-aggregate in C++11, so `{path, format, shift}`
    // would not compile in C++11 compatibility mode.
    struct Source {
        std::string path;
        InputFormat format;
        int64_t     shift_us;

        Source() NOEXCEPT : path(), format(INPUT_FORMAT_RAW), shift_us(0) {}
        Source(std::string p, InputFormat f = INPUT_FORMAT_RAW, int64_t shift = 0) NOEXCEPT
            : path(std::move(p)),
              format(f),
              shift_us(shift) {}
    };

    std::function<void(TbinInput&, InputFormat, uint8_t*, size_t)> format_callback;

    void set_stop_time_us(int64_t us) { mStopTimeUs = us; }

    explicit TbinInput(std::vector<Source> sources, bool replay_realtime = false) NOEXCEPT;
    ~TbinInput() NOEXCEPT override;

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler&) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler&) NOEXCEPT override;

private:
    void deliver_next() NOEXCEPT;

    struct Entry {
        int64_t  timestamp_us;
        uint32_t reader_index;
        bool     operator>(Entry const& o) const { return timestamp_us > o.timestamp_us; }
    };

    std::vector<format::tbin::Reader>                                   mReaders;
    std::vector<format::tbin::Message>                                  mPending;
    std::vector<InputFormat>                                            mFormats;
    std::vector<int64_t>                                                mShifts;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> mHeap;

    bool    mRealtimeMode;
    int64_t mStopTimeUs       = 0;
    int64_t mFirstTimestampUs = 0;
    int64_t mStartWallUs      = 0;
    int64_t mLastLogUs        = 0;
    bool    mStarted          = false;

    scheduler::RepeatableTimeoutTask mTask;
};
