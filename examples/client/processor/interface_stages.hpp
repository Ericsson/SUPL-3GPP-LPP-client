#pragma once
#include <client-io/input_format.hpp>
#include <client-io/stage.hpp>
#include <io/input.hpp>
#include <io/output.hpp>

#include <memory>

class InterfaceOutputStage : public OutputStage {
public:
    explicit InterfaceOutputStage(std::unique_ptr<io::Output> interface) NOEXCEPT
        : mInterface(std::move(interface)) {}

    void write(OutputFormat, uint8_t const* buffer, size_t length) NOEXCEPT override {
        mInterface->write(buffer, length);
    }

protected:
    bool do_schedule(scheduler::Scheduler& s) NOEXCEPT override { return mInterface->schedule(s); }
    bool do_cancel(scheduler::Scheduler&) NOEXCEPT override {
        mInterface->cancel();
        return true;
    }

private:
    std::unique_ptr<io::Output> mInterface;
};

class InterfaceInputStage : public InputStage {
public:
    explicit InterfaceInputStage(std::unique_ptr<io::Input> interface, InputFormat formats) NOEXCEPT
        : mInterface(std::move(interface)),
          mInputFormats(formats) {}

protected:
    bool do_schedule(scheduler::Scheduler& s) NOEXCEPT override {
        mInterface->callback = [this](io::Input&, uint8_t* data, size_t len) {
            if (callback) callback(mInputFormats, data, len);
        };
        return mInterface->schedule(s);
    }
    bool do_cancel(scheduler::Scheduler&) NOEXCEPT override {
        mInterface->cancel();
        return true;
    }

private:
    std::unique_ptr<io::Input> mInterface;
    InputFormat                mInputFormats;
};
