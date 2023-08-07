#include "serial.h"

#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

SerialTarget::SerialTarget(std::string device, const int baud_rate)
    : mDevice(std::move(device)), mBaudRate(baud_rate) {
    mFileDescriptor = open(mDevice.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (mFileDescriptor < 0) {
        throw std::runtime_error("Could not open serial device");
    }

    // set file status flags
    fcntl(mFileDescriptor, F_SETFL, 0);

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(mFileDescriptor, &tty) != 0) {
        throw std::runtime_error("Could not get serial device attributes");
    }

    // set baud rate
    cfsetospeed(&tty, (speed_t)mBaudRate);
    cfsetispeed(&tty, (speed_t)mBaudRate);

    // set raw mode
    cfmakeraw(&tty);

    /*

    if ((mode&STR_MODE_R)&&(mode&STR_MODE_W)) rw=O_RDWR;
    else if (mode&STR_MODE_R) rw=O_RDONLY;
    else if (mode&STR_MODE_W) rw=O_WRONLY;

    if ((serial->dev=open(dev,rw|O_NOCTTY|O_NONBLOCK))<0) {
        sprintf(msg,"%s open error (%d)",dev,errno);
        tracet(1,"openserial: %s dev=%s\n",msg,dev);
        free(serial);
        return NULL;
    }
    tcgetattr(serial->dev,&ios);
    ios.c_iflag=0;
    ios.c_oflag=0;
    ios.c_lflag=0;     // non-canonical
    ios.c_cc[VMIN ]=0; // non-block-mode
    ios.c_cc[VTIME]=0;
    cfsetospeed(&ios,bs[i]);
    cfsetispeed(&ios,bs[i]);
    ios.c_cflag|=bsize==7?CS7:CS8;
    ios.c_cflag|=parity=='O'?(PARENB|PARODD):(parity=='E'?PARENB:0);
    ios.c_cflag|=stopb==2?CSTOPB:0;
    ios.c_cflag|=!strcmp(fctr,"rts")?CRTSCTS:0;
    tcsetattr(serial->dev,TCSANOW,&ios);
    tcflush(serial->dev,TCIOFLUSH);
    sprintf(msg,"%s",dev);
    */

    tty.c_cflag |= (CLOCAL | CREAD);  // ignore modem controls,
                                      // enable receiver

    if (false) {
#if defined(CNEW_RTSCTS)
        tty.c_cflag |= CNEW_RTSCTS;  // enable RTS/CTS (hardware) flow control.
#elif defined(CRTSCTS)
        tty.c_cflag |= CRTSCTS;  // enable RTS/CTS (hardware) flow control.
#endif
    }

    if (tcsetattr(mFileDescriptor, TCSANOW, &tty) != 0) {
        throw std::runtime_error("Could not set serial device attributes");
    }

    tcflush(mFileDescriptor, TCIOFLUSH);
}

SerialTarget::~SerialTarget() {
    if (mFileDescriptor >= 0) {
        close(mFileDescriptor);
    }
}

void SerialTarget::transmit(const void* data, const size_t size) {
    if (mFileDescriptor < 0) {
        throw std::runtime_error("Serial device not open");
    }

    if (write(mFileDescriptor, data, size) != size) {
        throw std::runtime_error("Failed to write to serial device");
    }
}
