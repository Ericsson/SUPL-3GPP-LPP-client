#pragma once
#include <io/input.hpp>
#include <io/output.hpp>

#include <chrono>
#include <memory>
#include <string>

namespace scheduler {
class ForwardStreamTask;
class FileDescriptorTask;
}  // namespace scheduler

namespace io {

/// Data bits.
enum class DataBits {
    /// Five (5) data bits.
    FIVE,
    /// Six (6) data bits.
    SIX,
    /// Seven (7) data bits.
    SEVEN,
    /// Eight (8) data bits.
    EIGHT,
};

/// Stop bits.
enum class StopBits {
    /// One stop bit.
    ONE,
    /// Two stop bits.
    TWO,
};

/// Parity bit.
enum class ParityBit {
    /// No parity bit.
    NONE,
    /// Odd parity bit.
    ODD,
    /// Even parity bit.
    EVEN,
};

/// Baud rates.
enum class BaudRate {
    /// 50 baud.
    BR50,
    /// 75 baud.
    BR75,
    /// 110 baud.
    BR110,
    /// 134.5 baud.
    BR134,
    /// 150 baud.
    BR150,
    /// 200 baud.
    BR200,
    /// 300 baud.
    BR300,
    /// 600 baud.
    BR600,
    /// 1200 baud.
    BR1200,
    /// 1800 baud.
    BR1800,
    /// 2400 baud.
    BR2400,
    /// 4800 baud.
    BR4800,
    /// 9600 baud.
    BR9600,
    /// 19200 baud.
    BR19200,
    /// 38400 baud.
    BR38400,
    /// 57600 baud.
    BR57600,
    /// 115200 baud.
    BR115200,
    /// 230400 baud.
    BR230400,
    /// 460800 baud.
    BR460800,
    /// 500000 baud.
    BR500000,
    /// 576000 baud.
    BR576000,
    /// 921600 baud.
    BR921600,
    /// 1000000 baud.
    BR1000000,
    /// 1152000 baud.
    BR1152000,
    /// 1500000 baud.
    BR1500000,
    /// 2000000 baud.
    BR2000000,
    /// 2500000 baud.
    BR2500000,
    /// 3000000 baud.
    BR3000000,
    /// 3500000 baud.
    BR3500000,
    /// 4000000 baud.
    BR4000000,
};

char const* baud_rate_to_str(BaudRate baud_rate);
char const* data_bits_to_str(DataBits data_bits);
char const* stop_bits_to_str(StopBits stop_bits);
char const* parity_bit_to_str(ParityBit parity_bit);

/// Input from a serial port.
class SerialInput : public Input {
public:
    EXPLICIT SerialInput(std::string device, BaudRate baud_rate, DataBits data_bits,
                         StopBits stop_bits, ParityBit parity_bit) NOEXCEPT;
    ~SerialInput() NOEXCEPT override;

    NODISCARD std::string const& device() const NOEXCEPT { return mDevice; }
    NODISCARD BaudRate           baud_rate() const NOEXCEPT { return mBaudRate; }
    NODISCARD DataBits           data_bits() const NOEXCEPT { return mDataBits; }
    NODISCARD StopBits           stop_bits() const NOEXCEPT { return mStopBits; }
    NODISCARD ParityBit          parity_bit() const NOEXCEPT { return mParityBit; }

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    std::string mDevice;
    BaudRate    mBaudRate;
    DataBits    mDataBits;
    StopBits    mStopBits;
    ParityBit   mParityBit;

    int                                            mFd;
    std::unique_ptr<scheduler::FileDescriptorTask> mFdTask;

    uint8_t mBuffer[4096];
};

/// Output to a serial port.
class SerialOutput : public Output {
public:
    EXPLICIT SerialOutput(std::string device, BaudRate baud_rate, DataBits data_bits,
                          StopBits stop_bits, ParityBit parity_bit) NOEXCEPT;
    ~SerialOutput() NOEXCEPT override;

    void write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::string const& device() const NOEXCEPT { return mDevice; }
    NODISCARD BaudRate           baud_rate() const NOEXCEPT { return mBaudRate; }
    NODISCARD DataBits           data_bits() const NOEXCEPT { return mDataBits; }
    NODISCARD StopBits           stop_bits() const NOEXCEPT { return mStopBits; }
    NODISCARD ParityBit          parity_bit() const NOEXCEPT { return mParityBit; }

protected:
    void open();
    void close();

private:
    std::string mDevice;
    BaudRate    mBaudRate;
    DataBits    mDataBits;
    StopBits    mStopBits;
    ParityBit   mParityBit;

    int mFd;
};
}  // namespace io
