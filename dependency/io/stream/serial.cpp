#include <io/stream/serial.hpp>
#include <scheduler/socket.hpp>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE3(io, stream, serial);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(io, stream, serial)

namespace io {

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
    }
    return B115200;
}

SerialStream::SerialStream(std::string id, SerialConfig config) NOEXCEPT
    : Stream(std::move(id), config.read_config),
      mConfig(std::move(config)) {
    VSCOPE_FUNCTIONF("\"%s\", \"%s\", raw=%d", mId.c_str(), mConfig.device.c_str(), mConfig.raw);
}

SerialStream::~SerialStream() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool SerialStream::schedule(scheduler::Scheduler& scheduler) {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    mScheduler = &scheduler;

    mFd = ::open(mConfig.device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    VERBOSEF("::open(\"%s\", O_RDWR | O_NOCTTY | O_NONBLOCK) = %d", mConfig.device.c_str(), mFd);
    if (mFd < 0) {
        ERRORF("failed to open serial device \"%s\": " ERRNO_FMT, mConfig.device.c_str(),
               ERRNO_ARGS(errno));
        set_error(errno, "failed to open serial device");
        return false;
    }

    if (!mConfig.raw && !configure_termios()) {
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
        return false;
    }

    mSocketTask.reset(new scheduler::SocketTask(mFd));
    mSocketTask->set_event_name("serial:" + mId);
    mSocketTask->on_read = [this](scheduler::SocketTask&) {
        auto result = ::read(mFd, mReadBuf, sizeof(mReadBuf));
        VERBOSEF("::read(%d, %p, %zu) = %zd", mFd, mReadBuf, sizeof(mReadBuf), result);
        if (result > 0) {
            on_raw_read(mReadBuf, result);
        } else if (result == 0) {
            DEBUGF("serial device closed");
            set_disconnected();
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            ERRORF("failed to read from serial: " ERRNO_FMT, ERRNO_ARGS(errno));
            set_error(errno, strerror(errno));
        }
    };
    mSocketTask->on_write = [this](scheduler::SocketTask&) {
        while (!mWriteBuffer.empty()) {
            auto  peek   = mWriteBuffer.peek();
            auto& data   = peek.first;
            auto& len    = peek.second;
            auto  result = ::write(mFd, data, len);
            VERBOSEF("::write(%d, %p, %zu) = %zd", mFd, data, len, result);
            if (result < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                WARNF("write error: " ERRNO_FMT, ERRNO_ARGS(errno));
                return;
            }
            mWriteBuffer.consume(result);
        }
        if (mWriteBuffer.empty() && mWriteRegistered) {
            mScheduler->update_epoll_fd(mFd, EPOLLIN, nullptr);
            mWriteRegistered = false;
        }
    };
    mSocketTask->on_error = [this](scheduler::SocketTask&) {
        ERRORF("serial socket error: " ERRNO_FMT, ERRNO_ARGS(errno));
        set_error(errno, strerror(errno));
    };

    if (!mSocketTask->schedule(scheduler)) {
        ERRORF("failed to schedule socket task");
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
        return false;
    }

    if (!schedule_read_timeout(scheduler)) {
        mSocketTask->cancel();
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
        return false;
    }

    mState = State::Connected;
    DEBUGF("serial stream connected: %s", mConfig.device.c_str());
    return true;
}

bool SerialStream::cancel() {
    VSCOPE_FUNCTION();
    cancel_read_timeout();
    if (mSocketTask) {
        mSocketTask->cancel();
        mSocketTask.reset();
    }
    if (mFd >= 0) {
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
    }
    return true;
}

void SerialStream::write(uint8_t const* data, size_t length) NOEXCEPT {
    TRACEF("%p, %zu", data, length);

    if (mWriteBuffer.empty()) {
        auto result = ::write(mFd, data, length);
        VERBOSEF("::write(%d, %p, %zu) = %zd", mFd, data, length, result);
        if (result < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                result = 0;
            } else {
                WARNF("write error: " ERRNO_FMT, ERRNO_ARGS(errno));
                return;
            }
        }
        if (static_cast<size_t>(result) == length) return;
        data += result;
        length -= result;
    }

    mWriteBuffer.enqueue(data, length);
    if (!mWriteRegistered && mScheduler) {
        mScheduler->update_epoll_fd(mFd, EPOLLIN | EPOLLOUT, nullptr);
        mWriteRegistered = true;
    }
}

bool SerialStream::configure_termios() NOEXCEPT {
    VSCOPE_FUNCTION();

    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    auto result = ::tcgetattr(mFd, &tty);
    VERBOSEF("::tcgetattr(%d, %p) = %d", mFd, &tty, result);
    if (result != 0) {
        ERRORF("failed to get serial attributes: " ERRNO_FMT, ERRNO_ARGS(errno));
        set_error(errno, "failed to get serial attributes");
        return false;
    }

    ::cfmakeraw(&tty);
    VERBOSEF("::cfmakeraw(%p)", &tty);

    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN]  = 0;

    auto iflag = static_cast<int>(tty.c_iflag);
    iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_iflag = static_cast<tcflag_t>(iflag);

    tty.c_oflag &= ~OPOST;

    auto lflag = static_cast<int>(tty.c_lflag);
    lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_lflag = static_cast<tcflag_t>(lflag);

    auto cflag = static_cast<int>(tty.c_cflag);
    cflag |= (CLOCAL | CREAD);
    cflag &= ~(CRTSCTS | CSIZE | CSTOPB | PARENB | PARODD);

    switch (mConfig.data_bits) {
    case DataBits::FIVE: cflag |= CS5; break;
    case DataBits::SIX: cflag |= CS6; break;
    case DataBits::SEVEN: cflag |= CS7; break;
    case DataBits::EIGHT: cflag |= CS8; break;
    }

    if (mConfig.stop_bits == StopBits::TWO) cflag |= CSTOPB;

    switch (mConfig.parity_bit) {
    case ParityBit::NONE: break;
    case ParityBit::ODD: cflag |= (PARENB | PARODD); break;
    case ParityBit::EVEN: cflag |= PARENB; break;
    }

    tty.c_cflag = static_cast<tcflag_t>(cflag);

    auto baud = baud_rate_to_constant(mConfig.baud_rate);
    result    = ::cfsetospeed(&tty, baud);
    VERBOSEF("::cfsetospeed(%p, %d) = %d", &tty, baud, result);
    result = ::cfsetispeed(&tty, baud);
    VERBOSEF("::cfsetispeed(%p, %d) = %d", &tty, baud, result);

    result = ::tcsetattr(mFd, TCSANOW, &tty);
    VERBOSEF("::tcsetattr(%d, TCSANOW, %p) = %d", mFd, &tty, result);
    if (result != 0) {
        ERRORF("failed to set serial attributes: " ERRNO_FMT, ERRNO_ARGS(errno));
        set_error(errno, "failed to set serial attributes");
        return false;
    }

    return true;
}

}  // namespace io
