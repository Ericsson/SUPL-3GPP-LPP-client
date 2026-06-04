#pragma once
#include <chrono>
#include <cstring>
#include <io/output.hpp>
#include <memory>
#include <string>
#include <vector>

class TbinOutput : public io::Output {
public:
    TbinOutput(std::unique_ptr<io::Output> inner, std::string const& stream_id) NOEXCEPT
        : mInner(std::move(inner)),
          mHeaderWritten(false),
          mStreamId(stream_id) {}

    NODISCARD const char* name() const NOEXCEPT override { return "tbin"; }

    bool do_schedule(scheduler::Scheduler& s) NOEXCEPT override { return mInner->schedule(s); }
    bool do_cancel(scheduler::Scheduler&) NOEXCEPT override { return mInner->cancel(); }

    void write(uint8_t const* buffer, size_t length) NOEXCEPT override {
        if (!mHeaderWritten) {
            write_header();
            mHeaderWritten = true;
        }

        auto now = std::chrono::system_clock::now();
        auto us =
            std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
        auto ts  = static_cast<int64_t>(us);
        auto len = static_cast<uint32_t>(length);

        // Write as single buffer to avoid TCP fragmentation
        std::vector<uint8_t> frame(12 + length);
        memcpy(frame.data(), &ts, 8);
        memcpy(frame.data() + 8, &len, 4);
        memcpy(frame.data() + 12, buffer, length);
        mInner->write(frame.data(), frame.size());
    }

private:
    void write_header() {
        uint8_t header[4 + 1 + 1 + 64];
        size_t  pos   = 0;
        header[pos++] = 'T';
        header[pos++] = 'B';
        header[pos++] = 'I';
        header[pos++] = 'N';
        header[pos++] = 1;  // version
        auto id_len   = std::min(mStreamId.size(), size_t{63});
        header[pos++] = static_cast<uint8_t>(id_len);
        memcpy(header + pos, mStreamId.data(), id_len);
        pos += id_len;
        mInner->write(header, pos);
    }

    std::unique_ptr<io::Output> mInner;
    bool                        mHeaderWritten;
    std::string                 mStreamId;
};
