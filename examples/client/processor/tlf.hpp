#pragma once
#include <memory>
#include <queue>

#include <format/helper/parser.hpp>
#include <scheduler/periodic.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"
#include "stage.hpp"

class InterfaceOutputStage : public OutputStage {
public:
    EXPLICIT InterfaceOutputStage(std::unique_ptr<io::Output> interface) NOEXCEPT;
    ~InterfaceOutputStage() NOEXCEPT;

    void write(OutputFormat format, uint8_t const* buffer, size_t length) NOEXCEPT override;

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::unique_ptr<io::Output> mInterface;
};

class TlfOutputStage : public OutputStage {
public:
    EXPLICIT TlfOutputStage(std::unique_ptr<OutputStage> next) NOEXCEPT;
    ~TlfOutputStage() NOEXCEPT override;

    void write(OutputFormat format, uint8_t const* buffer, size_t length) NOEXCEPT override;

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::unique_ptr<OutputStage>          mNext;
    std::chrono::steady_clock::time_point mLastMessage;
    uint32_t                              mSequence = 0;
};

class InterfaceInputStage : public InputStage {
public:
    EXPLICIT InterfaceInputStage(std::unique_ptr<io::Input> interface,
                                 InputFormat                formats) NOEXCEPT;
    ~InterfaceInputStage() NOEXCEPT override;

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::unique_ptr<io::Input> mInterface;
    InputFormat                mInputFormats;
};

#define TLF_HEADER_LENGTH 24
struct TlfHeader {
    char     magic[4];
    uint32_t sequence;
    uint64_t format;
    uint32_t ms;
    uint32_t length;
};

static_assert(sizeof(TlfHeader) == TLF_HEADER_LENGTH, "TlfHeader size mismatch");

class TlfParser : public format::helper::Parser {
public:
    TlfParser() NOEXCEPT;
    ~TlfParser() NOEXCEPT override;

    char const* name() const NOEXCEPT override { return "TlfParser"; }

    NODISCARD bool parse(TlfHeader& header, std::vector<uint8_t>& output) NOEXCEPT;

private:
    TlfHeader mHeader;
    size_t    mBytesWanted = 0;
};

class TlfInputStage : public InputStage {
public:
    EXPLICIT TlfInputStage(std::unique_ptr<InputStage> parent) NOEXCEPT;
    ~TlfInputStage() NOEXCEPT override;

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

    void read(InputFormat format, uint8_t const* buffer, size_t length) NOEXCEPT;

    struct QueueItem {
        std::chrono::steady_clock::time_point time;
        InputFormat                           format;
        std::vector<uint8_t>                  buffer;
    };

private:
    std::unique_ptr<InputStage>           mParent;
    std::queue<QueueItem>                 mQueue;
    scheduler::PeriodicTask               mTask;
    TlfParser                             mParser;
    std::chrono::steady_clock::time_point mLastMessage;
};
