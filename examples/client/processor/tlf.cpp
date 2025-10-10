#include "tlf.hpp"

#include <loglet/loglet.hpp>
#include <string.h>

LOGLET_MODULE2(p, tlf);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, tlf)

constexpr static char MAGIC[4] = "TLF";

OutputStage::OutputStage() NOEXCEPT : mScheduler(nullptr) {}
OutputStage::~OutputStage() = default;

bool OutputStage::schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    if (mScheduler) {
        return false;
    }

    if (do_schedule(scheduler)) {
        mScheduler = &scheduler;
        return true;
    } else {
        return false;
    }
}

bool OutputStage::cancel() NOEXCEPT {
    if (!mScheduler) {
        return false;
    }

    if (do_cancel(*mScheduler)) {
        mScheduler = nullptr;
        return true;
    } else {
        return false;
    }
}

InterfaceOutputStage::InterfaceOutputStage(std::unique_ptr<io::Output> interface) NOEXCEPT
    : mInterface(std::move(interface)) {
    FUNCTION_SCOPE();
}
InterfaceOutputStage::~InterfaceOutputStage() NOEXCEPT {
    FUNCTION_SCOPE();
}

bool InterfaceOutputStage::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    return mInterface->schedule(scheduler);
}

bool InterfaceOutputStage::do_cancel(scheduler::Scheduler&) NOEXCEPT {
    return mInterface->cancel();
}

void InterfaceOutputStage::write(OutputFormat, uint8_t const* buffer, size_t length) NOEXCEPT {
    FUNCTION_SCOPE();
    mInterface->write(buffer, length);
}

TlfOutputStage::TlfOutputStage(std::unique_ptr<OutputStage> next) NOEXCEPT
    : mNext(std::move(next)) {
    FUNCTION_SCOPE();
}

TlfOutputStage::~TlfOutputStage() NOEXCEPT {
    FUNCTION_SCOPE();
}

bool TlfOutputStage::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    return mNext->schedule(scheduler);
}

bool TlfOutputStage::do_cancel(scheduler::Scheduler&) NOEXCEPT {
    return mNext->cancel();
}

void TlfOutputStage::write(OutputFormat format, uint8_t const* buffer, size_t length) NOEXCEPT {
    if (!mNext) {
        return;
    }

    FUNCTION_SCOPE();
    // Compute seconds since last message (0 = first message).
    // Output binary write MAGIC, then 4 bytes sequence, then 4 bytes seconds, then 8 bytes for
    // format, then 4 bytes length.

    auto now  = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - mLastMessage);
    if (mSequence == 0) {
        diff = std::chrono::seconds(0);
    }

    auto diff_us     = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
    auto diff_us_u32 = static_cast<uint32_t>(diff_us);

    auto header = TlfHeader{
        .magic    = {MAGIC[0], MAGIC[1], MAGIC[2], MAGIC[3]},
        .sequence = mSequence,
        .format   = format,
        .ms       = diff_us_u32,
        .length   = static_cast<uint32_t>(length),
    };
    mNext->write(OUTPUT_FORMAT_TLF, reinterpret_cast<uint8_t const*>(&header), sizeof(header));
    mNext->write(OUTPUT_FORMAT_TLF, buffer, length);

    mLastMessage = now;
    mSequence++;
}

//
//
//

InputStage::InputStage() NOEXCEPT : mScheduler(nullptr) {}
InputStage::~InputStage() = default;

bool InputStage::schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    if (mScheduler) {
        return false;
    }

    if (do_schedule(scheduler)) {
        mScheduler = &scheduler;
        return true;
    } else {
        return false;
    }
}
bool InputStage::cancel() NOEXCEPT {
    if (!mScheduler) {
        return false;
    }

    if (do_cancel(*mScheduler)) {
        mScheduler = nullptr;
        return true;
    } else {
        return false;
    }
}

InterfaceInputStage::InterfaceInputStage(std::unique_ptr<io::Input> interface,
                                         InputFormat                formats) NOEXCEPT
    : mInterface(std::move(interface)),
      mInputFormats(formats) {
    FUNCTION_SCOPE();
}
InterfaceInputStage::~InterfaceInputStage() NOEXCEPT {
    FUNCTION_SCOPE();
}

bool InterfaceInputStage::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    mInterface->callback = [this](io::Input&, uint8_t const* buffer, size_t length) {
        FUNCTION_SCOPEF("interface callback, %p, %zu", buffer, length);
        if (this->callback) {
            this->callback(mInputFormats, const_cast<uint8_t*>(buffer), length);
        }
    };

    return mInterface->schedule(scheduler);
}

bool InterfaceInputStage::do_cancel(scheduler::Scheduler&) NOEXCEPT {
    return mInterface->cancel();
}

TlfInputStage::TlfInputStage(std::unique_ptr<InputStage> parent) NOEXCEPT
    : mParent(std::move(parent)),
      mTask(std::chrono::milliseconds(100)) {
    FUNCTION_SCOPE();
    mLastMessage = std::chrono::steady_clock::now();

    mTask.set_event_name("TlfInputStage");
    mTask.callback = [this]() {
        FUNCTION_SCOPEF("periodic task");
        DEBUGF("queue size = %zu", mQueue.size());
        auto now = std::chrono::steady_clock::now();
        while (mQueue.size() > 0) {
            auto& item = mQueue.front();
            if (item.time <= now) {
                read(item.format, item.buffer.data(), item.buffer.size());
                mQueue.pop();
            } else {
                break;
            }
        }
    };

    mParent->callback = [this](InputFormat format, uint8_t const* buffer, size_t length) {
        FUNCTION_SCOPEF("callback, %016" PRIx64 ", %p, %zu", format, buffer, length);

        mParser.append(buffer, length);

        std::vector<uint8_t> output;
        TlfHeader            header;
        for (;;) {
            if (!mParser.parse(header, output)) {
                break;
            }

            auto next_time = mLastMessage + std::chrono::milliseconds(header.ms);
            mQueue.push({next_time, header.format, std::move(output)});
            DEBUGF("added message at %f", next_time.time_since_epoch().count() / 1.0e9);
        }
    };
}

TlfInputStage::~TlfInputStage() NOEXCEPT {
    FUNCTION_SCOPE();
}

bool TlfInputStage::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    return mParent->schedule(scheduler);
}

bool TlfInputStage::do_cancel(scheduler::Scheduler&) NOEXCEPT {
    return mParent->cancel();
}

void TlfInputStage::read(InputFormat format, uint8_t const* buffer, size_t length) NOEXCEPT {
    FUNCTION_SCOPE();

    if (callback) {
        callback(format, const_cast<uint8_t*>(buffer), length);
    }
}

TlfParser::TlfParser() NOEXCEPT : mBytesWanted(0) {
    FUNCTION_SCOPE();
}

TlfParser::~TlfParser() NOEXCEPT {
    FUNCTION_SCOPE();
}

bool TlfParser::parse(TlfHeader& header, std::vector<uint8_t>& output) NOEXCEPT {
    FUNCTION_SCOPE();

    if (mBytesWanted == 0) {
        if (buffer_length() < sizeof(TlfHeader)) {
            VERBOSEF("buffer too small: %zu < %zu", buffer_length(), sizeof(TlfHeader));
            return false;
        }

        copy_to_buffer(reinterpret_cast<uint8_t*>(&mHeader), sizeof(mHeader));
        if (strncmp(mHeader.magic, MAGIC, sizeof(MAGIC)) != 0) {
            VERBOSEF("invalid magic: %s", mHeader.magic);
            skip(sizeof(TlfHeader));
            return false;
        }
        skip(sizeof(TlfHeader));
        mBytesWanted = mHeader.length;
    }

    if (buffer_length() < mBytesWanted) {
        VERBOSEF("buffer too small: %zu < %zu", buffer_length(), mBytesWanted);
        return false;
    }

    output.resize(mBytesWanted);
    copy_to_buffer(output.data(), mBytesWanted);
    skip(mBytesWanted);
    header       = mHeader;
    mBytesWanted = 0;
    return true;
}
