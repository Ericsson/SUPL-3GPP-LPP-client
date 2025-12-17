#pragma once
#include <io/serial.hpp>
#include <io/stream.hpp>
#include <io/write_buffer.hpp>

#include <memory>
#include <string>

namespace scheduler {
class SocketTask;
}

namespace io {

struct SerialConfig {
    std::string      device;
    BaudRate         baud_rate   = BaudRate::BR115200;
    DataBits         data_bits   = DataBits::EIGHT;
    StopBits         stop_bits   = StopBits::ONE;
    ParityBit        parity_bit  = ParityBit::NONE;
    bool             raw         = false;
    ReadBufferConfig read_config = {};
};

class SerialStream : public Stream {
public:
    EXPLICIT SerialStream(std::string id, SerialConfig config) NOEXCEPT;
    ~SerialStream() NOEXCEPT override;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) override;
    bool           cancel() override;
    void           write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::string const& device() const NOEXCEPT { return mConfig.device; }
    NODISCARD size_t pending_writes() const NOEXCEPT override { return mWriteBuffer.size(); }

private:
    bool configure_termios() NOEXCEPT;

    SerialConfig                           mConfig;
    int                                    mFd = -1;
    std::unique_ptr<scheduler::SocketTask> mSocketTask;
    WriteBuffer                            mWriteBuffer;
    bool                                   mWriteRegistered = false;
    uint8_t                                mReadBuf[4096];
};

}  // namespace io
