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

inline const char* data_bits_to_string(DataBits data_bits) NOEXCEPT {
    switch (data_bits) {
        case DataBits::FIVE: return "5";
        case DataBits::SIX: return "6";
        case DataBits::SEVEN: return "7";
        case DataBits::EIGHT: return "8";
    }
    return "unknown";
}

/// Stop bits.
enum class StopBits {
    /// One stop bit.
    ONE,
    /// Two stop bits.
    TWO,
};

inline const char* stop_bits_to_string(StopBits stop_bits) NOEXCEPT {
    switch (stop_bits) {
        case StopBits::ONE: return "1";
        case StopBits::TWO: return "2";
    }
    return "unknown";
}

/// Parity bit.
enum class ParityBit {
    /// No parity bit.
    NONE,
    /// Odd parity bit.
    ODD,
    /// Even parity bit.
    EVEN,
};

inline const char* parity_bit_to_string(ParityBit parity_bit) NOEXCEPT {
    switch (parity_bit) {
        case ParityBit::NONE: return "none";
        case ParityBit::ODD: return "odd";
        case ParityBit::EVEN: return "even";
    }
    return "unknown";
}

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

inline const char* baud_rate_to_string(BaudRate baud_rate) NOEXCEPT {
    switch (baud_rate) {
        case BaudRate::BR50: return "50";
        case BaudRate::BR75: return "75";
        case BaudRate::BR110: return "110";
        case BaudRate::BR134: return "134";
        case BaudRate::BR150: return "150";
        case BaudRate::BR200: return "200";
        case BaudRate::BR300: return "300";
        case BaudRate::BR600: return "600";
        case BaudRate::BR1200: return "1200";
        case BaudRate::BR1800: return "1800";
        case BaudRate::BR2400: return "2400";
        case BaudRate::BR4800: return "4800";
        case BaudRate::BR9600: return "9600";
        case BaudRate::BR19200: return "19200";
        case BaudRate::BR38400: return "38400";
        case BaudRate::BR57600: return "57600";
        case BaudRate::BR115200: return "115200";
        case BaudRate::BR230400: return "230400";
        case BaudRate::BR460800: return "460800";
        case BaudRate::BR500000: return "500000";
        case BaudRate::BR576000: return "576000";
        case BaudRate::BR921600: return "921600";
        case BaudRate::BR1000000: return "1000000";
        case BaudRate::BR1152000: return "1152000";
        case BaudRate::BR1500000: return "1500000";
        case BaudRate::BR2000000: return "2000000";
        case BaudRate::BR2500000: return "2500000";
        case BaudRate::BR3000000: return "3000000";
        case BaudRate::BR3500000: return "3500000";
        case BaudRate::BR4000000: return "4000000";
    }
    return "unknown";
}

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
