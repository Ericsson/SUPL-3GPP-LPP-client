#include <doctest/doctest.h>
#include <io/stream/pty.hpp>
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

TEST_CASE("PtyStream - loopback") {
    scheduler::Scheduler sched;
    io::PtyConfig        config;
    io::PtyStream        stream("test-pty", config);

    REQUIRE(stream.schedule(sched));
    REQUIRE_FALSE(stream.slave_path().empty());

    int slave_fd = ::open(stream.slave_path().c_str(), O_RDWR | O_NOCTTY);
    REQUIRE(slave_fd >= 0);
    set_raw_mode(slave_fd);

    std::vector<uint8_t> received;
    stream.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        received.insert(received.end(), data, data + len);
    });

    ::write(slave_fd, "test", 4);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return received.size() >= 4;
    }));
    CHECK(memcmp(received.data(), "test", 4) == 0);

    stream.write(reinterpret_cast<uint8_t const*>("reply"), 5);
    sched.yield();
    sched.execute_once();

    char buf[10];
    auto n = ::read(slave_fd, buf, sizeof(buf));
    REQUIRE(n == 5);
    CHECK(memcmp(buf, "reply", 5) == 0);

    ::close(slave_fd);
}

TEST_CASE("PtyStream - raw mode") {
    scheduler::Scheduler sched;
    io::PtyConfig        config;
    config.raw = true;
    io::PtyStream stream("test-pty-raw", config);

    REQUIRE(stream.schedule(sched));
    REQUIRE_FALSE(stream.slave_path().empty());

    int slave_fd = ::open(stream.slave_path().c_str(), O_RDWR | O_NOCTTY);
    REQUIRE(slave_fd >= 0);
    set_raw_mode(slave_fd);

    std::vector<uint8_t> received;
    stream.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        received.insert(received.end(), data, data + len);
    });

    ::write(slave_fd, "raw", 3);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return received.size() >= 3;
    }));
    CHECK(memcmp(received.data(), "raw", 3) == 0);

    ::close(slave_fd);
}

TEST_CASE("PtyStream - symlink creation") {
    scheduler::Scheduler sched;
    io::PtyConfig        config;
    config.link_path = "/tmp/test-pty-link";
    io::PtyStream stream("test-pty", config);

    REQUIRE(stream.schedule(sched));

    char link_target[256];
    auto len = ::readlink("/tmp/test-pty-link", link_target, sizeof(link_target) - 1);
    REQUIRE(len > 0);
    link_target[len] = '\0';
    CHECK(std::string(link_target) == stream.slave_path());

    stream.cancel();
    CHECK(::access("/tmp/test-pty-link", F_OK) != 0);
}
