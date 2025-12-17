#include <doctest/doctest.h>
#include <io/stream/fd.hpp>
#include <scheduler/scheduler.hpp>

#include "test_helper.hpp"

#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

TEST_CASE("FdStream - pipe loopback") {
    int fds[2];
    REQUIRE(pipe(fds) == 0);

    scheduler::Scheduler sched;
    io::FdConfig         config;
    config.fd      = fds[0];
    config.owns_fd = true;
    io::FdStream stream("test", config);

    std::vector<uint8_t> received;
    stream.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        received.insert(received.end(), data, data + len);
    });

    REQUIRE(stream.schedule(sched));

    ::write(fds[1], "hello", 5);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return received.size() >= 5;
    }));
    REQUIRE(received.size() == 5);
    CHECK(memcmp(received.data(), "hello", 5) == 0);

    ::close(fds[1]);
}

TEST_CASE("FdStream - write to pipe") {
    int fds[2];
    REQUIRE(pipe(fds) == 0);

    fcntl(fds[1], F_SETFL, fcntl(fds[1], F_GETFL, 0) | O_NONBLOCK);

    scheduler::Scheduler sched;
    io::FdConfig         config;
    config.fd      = fds[1];
    config.owns_fd = true;
    io::FdStream stream("test", config);

    REQUIRE(stream.schedule(sched));

    stream.write(reinterpret_cast<uint8_t const*>("world"), 5);
    sched.yield();
    sched.execute_once();

    char buf[10];
    auto n = ::read(fds[0], buf, sizeof(buf));
    REQUIRE(n == 5);
    CHECK(memcmp(buf, "world", 5) == 0);

    ::close(fds[0]);
}

TEST_CASE("FdStream - read buffering min_bytes") {
    int fds[2];
    REQUIRE(pipe(fds) == 0);

    scheduler::Scheduler sched;
    io::FdConfig         config;
    config.fd                    = fds[0];
    config.owns_fd               = true;
    config.read_config.min_bytes = 10;
    io::FdStream stream("test", config);

    std::vector<uint8_t> received;
    stream.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        received.insert(received.end(), data, data + len);
    });

    REQUIRE(stream.schedule(sched));

    ::write(fds[1], "hello", 5);
    sched.yield();
    sched.execute_once();
    CHECK(received.empty());

    ::write(fds[1], "world", 5);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return received.size() >= 10;
    }));
    CHECK(received.size() == 10);

    ::close(fds[1]);
}

TEST_CASE("FdStream - multiple read callbacks") {
    int fds[2];
    REQUIRE(pipe(fds) == 0);

    scheduler::Scheduler sched;
    io::FdConfig         config;
    config.fd      = fds[0];
    config.owns_fd = true;
    io::FdStream stream("test", config);

    int  count1 = 0, count2 = 0;
    auto h1 = stream.on_read([&](io::Stream&, uint8_t*, size_t) {
        count1++;
    });
    stream.on_read([&](io::Stream&, uint8_t*, size_t) {
        count2++;
    });

    REQUIRE(stream.schedule(sched));

    ::write(fds[1], "test", 4);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return count1 > 0 && count2 > 0;
    }));
    CHECK(count1 == 1);
    CHECK(count2 == 1);

    stream.remove_on_read(h1);
    ::write(fds[1], "more", 4);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return count2 > 1;
    }));
    CHECK(count1 == 1);
    CHECK(count2 == 2);

    ::close(fds[1]);
}

TEST_CASE("FdStream - high load write") {
    int fds[2];
    REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    fcntl(fds[0], F_SETFL, fcntl(fds[0], F_GETFL, 0) | O_NONBLOCK);
    fcntl(fds[1], F_SETFL, fcntl(fds[1], F_GETFL, 0) | O_NONBLOCK);

    scheduler::Scheduler sched;
    io::FdConfig         config;
    config.fd      = fds[0];
    config.owns_fd = true;
    io::FdStream stream("test", config);

    REQUIRE(stream.schedule(sched));

    std::vector<uint8_t> send_data(32 * 1024, 0xAB);
    stream.write(send_data.data(), send_data.size());

    std::vector<uint8_t> recv_data;
    char                 buf[4096];

    REQUIRE(run_until_or_timeout(sched, [&] {
        ssize_t n;
        while ((n = ::read(fds[1], buf, sizeof(buf))) > 0) {
            recv_data.insert(recv_data.end(), buf, buf + n);
        }
        return recv_data.size() >= send_data.size();
    }));

    CHECK(recv_data.size() == send_data.size());
    CHECK(recv_data == send_data);

    ::close(fds[1]);
}

TEST_CASE("FdStream - read buffering timeout") {
    int fds[2];
    REQUIRE(pipe(fds) == 0);

    scheduler::Scheduler sched;
    io::FdConfig         config;
    config.fd                    = fds[0];
    config.owns_fd               = true;
    config.read_config.min_bytes = 10;
    config.read_config.timeout   = std::chrono::milliseconds(100);
    io::FdStream stream("test", config);

    std::vector<uint8_t> received;
    stream.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        received.insert(received.end(), data, data + len);
    });

    REQUIRE(stream.schedule(sched));

    // Write less than min_bytes
    ::write(fds[1], "hello", 5);

    // Should not receive immediately (min_bytes not reached)
    sched.yield();
    sched.execute_once();
    CHECK(received.empty());

    // Wait for timeout to flush
    REQUIRE(run_until_or_timeout(sched, [&] {
        return !received.empty();
    }));
    CHECK(received.size() == 5);
    CHECK(memcmp(received.data(), "hello", 5) == 0);

    ::close(fds[1]);
}

TEST_CASE("FdStream - write buffer with small pipe") {
    int fds[2];
    REQUIRE(pipe(fds) == 0);

    // Set pipe to non-blocking and get a small buffer
    fcntl(fds[0], F_SETFL, fcntl(fds[0], F_GETFL, 0) | O_NONBLOCK);
    fcntl(fds[1], F_SETFL, fcntl(fds[1], F_GETFL, 0) | O_NONBLOCK);

    int buf_size = 4096;

    // Set small pipe buffer (Linux specific)
    fcntl(fds[1], F_SETPIPE_SZ, buf_size);

    scheduler::Scheduler sched;
    io::FdConfig         config;
    config.fd      = fds[1];
    config.owns_fd = true;
    io::FdStream stream("test", config);

    REQUIRE(stream.schedule(sched));

    int data_count = 8;  // 32KB total, fits in 64KB WriteBuffer
    int data_size  = data_count * buf_size;

    // Write more than pipe buffer can hold - this should buffer in WriteBuffer
    std::vector<uint8_t> large_data(data_size, 0xCD);
    stream.write(large_data.data(), large_data.size());

    // Some data should be pending in write buffer
    CHECK(stream.pending_writes() > 0);

    // Read from other end to drain and allow more writes
    std::vector<uint8_t> recv_data;
    char                 buf[4096];

    int recv_count = 0;
    REQUIRE(run_until_or_timeout(sched, [&] {
        ssize_t n;
        while ((n = ::read(fds[0], buf, sizeof(buf))) > 0) {
            recv_data.insert(recv_data.end(), buf, buf + n);
        }
        recv_count++;
        return recv_data.size() >= large_data.size();
    }));

    CHECK(recv_data.size() == large_data.size());
    CHECK(recv_data == large_data);
    CHECK(recv_count == data_count);
    CHECK(stream.pending_writes() == 0);

    ::close(fds[0]);
}

TEST_CASE("FdStream - on_complete on closed fd") {
    int fds[2];
    REQUIRE(pipe(fds) == 0);

    scheduler::Scheduler sched;
    io::FdConfig         config;
    config.fd      = fds[0];
    config.owns_fd = true;
    io::FdStream stream("test", config);

    bool completed     = false;
    stream.on_complete = [&]() {
        completed = true;
    };

    REQUIRE(stream.schedule(sched));

    ::close(fds[1]);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return completed;
    }));
    CHECK(completed);
}
