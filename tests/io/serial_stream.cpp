#include <doctest/doctest.h>
#include <io/stream/pty.hpp>
#include <io/stream/serial.hpp>
#include <scheduler/scheduler.hpp>

#include "test_helper.hpp"

#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

static void set_raw_mode(int fd) {
    struct termios tio;
    tcgetattr(fd, &tio);
    cfmakeraw(&tio);
    tcsetattr(fd, TCSANOW, &tio);
}

TEST_CASE("SerialStream - loopback via PTY") {
    scheduler::Scheduler sched;

    // Create PTY to get a slave path we can use as fake serial device
    io::PtyConfig pty_config;
    io::PtyStream pty("fake-serial-pty", pty_config);
    REQUIRE(pty.schedule(sched));
    REQUIRE_FALSE(pty.slave_path().empty());

    // Create SerialStream using the PTY slave as device (raw mode to skip termios)
    io::SerialConfig serial_config;
    serial_config.device = pty.slave_path();
    serial_config.raw    = true;
    io::SerialStream serial("test-serial", serial_config);
    REQUIRE(serial.schedule(sched));

    std::vector<uint8_t> received;
    serial.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        received.insert(received.end(), data, data + len);
    });

    // Write from PTY master, read on serial
    pty.write(reinterpret_cast<uint8_t const*>("hello"), 5);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return received.size() >= 5;
    }));
    REQUIRE(received.size() == 5);
    CHECK(memcmp(received.data(), "hello", 5) == 0);

    // Write from serial, read on PTY master
    std::vector<uint8_t> pty_received;
    pty.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        pty_received.insert(pty_received.end(), data, data + len);
    });

    serial.write(reinterpret_cast<uint8_t const*>("world"), 5);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return pty_received.size() >= 5;
    }));
    REQUIRE(pty_received.size() == 5);
    CHECK(memcmp(pty_received.data(), "world", 5) == 0);
}

TEST_CASE("SerialStream - configured termios via PTY") {
    scheduler::Scheduler sched;

    // Create PTY
    io::PtyConfig pty_config;
    io::PtyStream pty("fake-serial-pty", pty_config);
    REQUIRE(pty.schedule(sched));

    // Create SerialStream with termios configuration
    io::SerialConfig serial_config;
    serial_config.device     = pty.slave_path();
    serial_config.baud_rate  = io::BaudRate::BR115200;
    serial_config.data_bits  = io::DataBits::EIGHT;
    serial_config.stop_bits  = io::StopBits::ONE;
    serial_config.parity_bit = io::ParityBit::NONE;
    io::SerialStream serial("test-serial", serial_config);
    REQUIRE(serial.schedule(sched));

    std::vector<uint8_t> received;
    serial.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        received.insert(received.end(), data, data + len);
    });

    pty.write(reinterpret_cast<uint8_t const*>("cfg"), 3);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return received.size() >= 3;
    }));
    CHECK(memcmp(received.data(), "cfg", 3) == 0);
}
