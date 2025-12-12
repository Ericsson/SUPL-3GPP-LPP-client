
#include "serial.hpp"

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <termios.h>
#include <unistd.h>

#include <scheduler/file_descriptor.hpp>
#include <scheduler/scheduler.hpp>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(io, serial);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(io, serial)

namespace io {

char const* baud_rate_to_str(BaudRate baud_rate) {
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
    UNREACHABLE();
}

char const* data_bits_to_str(DataBits data_bits) {
    switch (data_bits) {
    case DataBits::FIVE: return "5";
    case DataBits::SIX: return "6";
    case DataBits::SEVEN: return "7";
    case DataBits::EIGHT: return "8";
    }
    UNREACHABLE();
}

char const* stop_bits_to_str(StopBits stop_bits) {
    switch (stop_bits) {
    case StopBits::ONE: return "1";
    case StopBits::TWO: return "2";
    }
    UNREACHABLE();
}

char const* parity_bit_to_str(ParityBit parity_bit) {
    switch (parity_bit) {
    case ParityBit::NONE: return "none";
    case ParityBit::ODD: return "odd";
    case ParityBit::EVEN: return "even";
    }
    UNREACHABLE();
}

SerialInput::SerialInput(std::string device, BaudRate baud_rate, DataBits data_bits,
                         StopBits stop_bits, ParityBit parity_bit) NOEXCEPT
    : mDevice(std::move(device)),
      mBaudRate(baud_rate),
      mDataBits(data_bits),
      mStopBits(stop_bits),
      mParityBit(parity_bit),
      mFd(-1) {
    VSCOPE_FUNCTIONF("\"%s\", %s, %s, %s, %s", mDevice.c_str(), baud_rate_to_str(mBaudRate),
                     data_bits_to_str(mDataBits), stop_bits_to_str(mStopBits),
                     parity_bit_to_str(mParityBit));

    std::stringstream ss;
    ss << "serial:" << mDevice;
    mEventName = ss.str();
}
SerialInput::~SerialInput() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

static speed_t baud_rate_to_constant(BaudRate baud_rate) {
    switch (baud_rate) {
    case BaudRate::BR50: return B50;
    case BaudRate::BR75: return B75;
    case BaudRate::BR110: return B110;
    case BaudRate::BR134: return B134;
    case BaudRate::BR150: return B150;
    case BaudRate::BR200: return B200;
    case BaudRate::BR300: return B300;
    case BaudRate::BR600: return B600;
    case BaudRate::BR1200: return B1200;
    case BaudRate::BR1800: return B1800;
    case BaudRate::BR2400: return B2400;
    case BaudRate::BR4800: return B4800;
    case BaudRate::BR9600: return B9600;
    case BaudRate::BR19200: return B19200;
    case BaudRate::BR38400: return B38400;
    case BaudRate::BR57600: return B57600;
    case BaudRate::BR115200: return B115200;
    case BaudRate::BR230400: return B230400;
    case BaudRate::BR460800: return B460800;
    case BaudRate::BR500000: return B500000;
    case BaudRate::BR576000: return B576000;
    case BaudRate::BR921600: return B921600;
    case BaudRate::BR1000000: return B1000000;
    case BaudRate::BR1152000: return B1152000;
    case BaudRate::BR1500000: return B1500000;
    case BaudRate::BR2000000: return B2000000;
    case BaudRate::BR2500000: return B2500000;
    case BaudRate::BR3000000: return B3000000;
    case BaudRate::BR3500000: return B3500000;
    case BaudRate::BR4000000: return B4000000;
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    default: UNREACHABLE(); break;
#endif
    }
}

bool SerialInput::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    mFd = ::open(mDevice.c_str(), O_RDONLY | O_NOCTTY | O_SYNC);
    VERBOSEF("::open(\"%s\", O_RDONLY | O_NOCTTY | O_SYNC) = %d", mDevice.c_str(), mFd);
    if (mFd < 0) {
        ERRORF("failed to open serial device \"%s\": " ERRNO_FMT, mDevice.c_str(),
               ERRNO_ARGS(errno));
        return false;
    }

    // set file status flags
    auto result = ::fcntl(mFd, F_SETFL, 0);
    VERBOSEF("::fcntl(%d, F_SETFL, 0) = %d", mFd, result);

    // set non-blocking
    auto flags = ::fcntl(mFd, F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", mFd, flags);
    result = ::fcntl(mFd, F_SETFL, flags | O_NONBLOCK);
    VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", mFd, flags | O_NONBLOCK, result);

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    result = ::tcgetattr(mFd, &tty);
    VERBOSEF("::tcgetattr(%d, %p) = %d", mFd, &tty, result);
    if (result != 0) {
        ERRORF("failed to get serial device attributes: " ERRNO_FMT, ERRNO_ARGS(errno));
        result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
        return false;
    }

    // set raw mode
    ::cfmakeraw(&tty);
    VERBOSEF("::cfmakeraw(%p)", &tty);

    // configure vtime and vmin as non-blocking
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN]  = 0;

    auto cflag = static_cast<int>(tty.c_cflag);
    cflag |= (CLOCAL | CREAD);    // ignore modem controls,
                                  // enable receiver
    cflag &= ~CRTSCTS;            // enable RTS/CTS (hardware) flow control.
    cflag &= ~CSIZE;              // mask the character size bits
    cflag &= ~CSTOPB;             // mask the stop bits
    cflag &= ~(PARENB | PARODD);  // mask parity bits

    switch (mDataBits) {
    case DataBits::FIVE: cflag |= CS5; break;
    case DataBits::SIX: cflag |= CS6; break;
    case DataBits::SEVEN: cflag |= CS7; break;
    case DataBits::EIGHT: cflag |= CS8; break;
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    default: UNREACHABLE(); break;
#endif
    }

    switch (mStopBits) {
    case StopBits::ONE: cflag &= ~CSTOPB; break;
    case StopBits::TWO: cflag |= CSTOPB; break;
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    default: UNREACHABLE(); break;
#endif
    }

    switch (mParityBit) {
    case ParityBit::NONE: cflag &= ~PARENB; break;
    case ParityBit::ODD:
        cflag |= PARENB;
        cflag |= PARODD;
        break;
    case ParityBit::EVEN:
        cflag |= PARENB;
        cflag &= ~PARODD;
        break;
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    default: UNREACHABLE(); break;
#endif
    }

    tty.c_cflag = static_cast<tcflag_t>(cflag);

    auto baud_rate_constant = baud_rate_to_constant(mBaudRate);

    // set baud rate
    result = ::cfsetospeed(&tty, baud_rate_constant);
    VERBOSEF("::cfsetospeed(%p, %d) = %d", &tty, baud_rate_constant, result);
    if (result != 0) {
        WARNF("failed to set serial output baud rate: " ERRNO_FMT, ERRNO_ARGS(errno));
    }

    result = ::cfsetispeed(&tty, baud_rate_constant);
    VERBOSEF("::cfsetispeed(%p, %d) = %d", &tty, baud_rate_constant, result);
    if (result != 0) {
        WARNF("failed to set serial input baud rate: " ERRNO_FMT, ERRNO_ARGS(errno));
    }

    result = ::tcsetattr(mFd, TCSANOW, &tty);
    VERBOSEF("::tcsetattr(%d, TCSANOW, %p) = %d", mFd, &tty, result);
    if (tcsetattr(mFd, TCSANOW, &tty) != 0) {
        ERRORF("failed to set serial device attributes: " ERRNO_FMT, ERRNO_ARGS(errno));
        result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
        return false;
    }

    mFdTask.reset(new scheduler::FileDescriptorTask());
    // Return value indicates rescheduling need, not failure - safe to ignore for new task
    (void)mFdTask->set_fd(mFd);

    mFdTask->set_event_name("fd/" + mEventName);
    mFdTask->on_read = [this](int) {
        while (true) {
            auto read_result = ::read(mFd, mBuffer, sizeof(mBuffer));
            VERBOSEF("::read(%d, %p, %zu) = %d", mFd, mBuffer, sizeof(mBuffer), read_result);
            if (read_result > 0) {
                if (callback) {
                    callback(*this, mBuffer, static_cast<size_t>(read_result));
                }
            } else if (read_result == 0) {
                TRACEF("read EOF");
                break;
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    TRACEF("read would block");
                    break;
                }
                ERRORF("read error: " ERRNO_FMT, ERRNO_ARGS(errno));
                break;
            }
        }
    };
    mFdTask->on_error = [this](int) {
        cancel();
        if (on_complete) on_complete();
    };

    if (!mFdTask->schedule(scheduler)) {
        result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
        return false;
    }

    return true;
}

bool SerialInput::do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    if (mFdTask) {
        mFdTask->cancel();
        mFdTask.reset();
    }

    if (mFd >= 0) {
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
    }

    return true;
}

//
//
//

SerialOutput::SerialOutput(std::string device, BaudRate baud_rate, DataBits data_bits,
                           StopBits stop_bits, ParityBit parity_bit) NOEXCEPT
    : mDevice(std::move(device)),
      mBaudRate(baud_rate),
      mDataBits(data_bits),
      mStopBits(stop_bits),
      mParityBit(parity_bit),
      mFd(-1) {
    VSCOPE_FUNCTIONF("\"%s\", %s, %s, %s, %s", mDevice.c_str(), baud_rate_to_str(mBaudRate),
                     data_bits_to_str(mDataBits), stop_bits_to_str(mStopBits),
                     parity_bit_to_str(mParityBit));
}

SerialOutput::~SerialOutput() NOEXCEPT {
    VSCOPE_FUNCTION();
    close();
}

void SerialOutput::write(uint8_t const* buffer, size_t length) NOEXCEPT {
    if (mFd < 0) {
        open();
    }

    if (mFd < 0) {
        return;
    }

    auto result = ::write(mFd, buffer, length);
    VERBOSEF("::write(%d, %p, %zu) = %d", mFd, buffer, length, result);
    if (result < 0) {
        WARNF("failed to write to serial device \"%s\": " ERRNO_FMT, mDevice.c_str(),
              ERRNO_ARGS(errno));
    }
}

void SerialOutput::open() {
    mFd = ::open(mDevice.c_str(), O_WRONLY | O_NOCTTY | O_SYNC);
    VERBOSEF("::open(\"%s\", O_WRONLY | O_NOCTTY | O_SYNC) = %d", mDevice.c_str(), mFd);
    if (mFd < 0) {
        WARNF("failed to open serial device \"%s\": " ERRNO_FMT, mDevice.c_str(),
              ERRNO_ARGS(errno));
        return;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    auto result = ::tcgetattr(mFd, &tty);
    VERBOSEF("::tcgetattr(%d, %p) = %d", mFd, &tty, result);
    if (result != 0) {
        WARNF("failed to get serial device attributes: " ERRNO_FMT, ERRNO_ARGS(errno));
        result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
        return;
    }

    // set raw mode
    ::cfmakeraw(&tty);
    VERBOSEF("::cfmakeraw(%p)", &tty);

    // configure vtime and vmin as non-blocking
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN]  = 0;

    auto cflag = static_cast<int>(tty.c_cflag);
    cflag |= (CLOCAL | CREAD);    // ignore modem controls,
                                  // enable receiver
    cflag &= ~CRTSCTS;            // enable RTS/CTS (hardware) flow control.
    cflag &= ~CSIZE;              // mask the character size bits
    cflag &= ~CSTOPB;             // mask the stop bits
    cflag &= ~(PARENB | PARODD);  // mask parity bits

    switch (mDataBits) {
    case DataBits::FIVE: cflag |= CS5; break;
    case DataBits::SIX: cflag |= CS6; break;
    case DataBits::SEVEN: cflag |= CS7; break;
    case DataBits::EIGHT: cflag |= CS8; break;
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    default: UNREACHABLE(); break;
#endif
    }

    switch (mStopBits) {
    case StopBits::ONE: cflag &= ~CSTOPB; break;
    case StopBits::TWO: cflag |= CSTOPB; break;
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    default: UNREACHABLE(); break;
#endif
    }

    switch (mParityBit) {
    case ParityBit::NONE: cflag &= ~PARENB; break;
    case ParityBit::ODD:
        cflag |= PARENB;
        cflag |= PARODD;
        break;
    case ParityBit::EVEN:
        cflag |= PARENB;
        cflag &= ~PARODD;
        break;
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    default: UNREACHABLE(); break;
#endif
    }

    tty.c_cflag = static_cast<tcflag_t>(cflag);

    auto baud_rate_constant = baud_rate_to_constant(mBaudRate);

    // set baud rate
    result = ::cfsetospeed(&tty, baud_rate_constant);
    VERBOSEF("::cfsetospeed(%p, %d) = %d", &tty, baud_rate_constant, result);
    if (result != 0) {
        WARNF("failed to set serial output baud rate: " ERRNO_FMT, ERRNO_ARGS(errno));
    }

    result = ::cfsetispeed(&tty, baud_rate_constant);
    VERBOSEF("::cfsetispeed(%p, %d) = %d", &tty, baud_rate_constant, result);
    if (result != 0) {
        WARNF("failed to set serial input baud rate: " ERRNO_FMT, ERRNO_ARGS(errno));
    }

    result = ::tcsetattr(mFd, TCSANOW, &tty);
    VERBOSEF("::tcsetattr(%d, TCSANOW, %p) = %d", mFd, &tty, result);
    if (tcsetattr(mFd, TCSANOW, &tty) != 0) {
        ERRORF("failed to set serial device attributes: " ERRNO_FMT, ERRNO_ARGS(errno));
        result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
        return;
    }
}

void SerialOutput::close() {
    if (mFd >= 0) {
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
    }
}
}  // namespace io
