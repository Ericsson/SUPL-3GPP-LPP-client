#include <doctest/doctest.h>
#include <io/adapters.hpp>
#include <io/stream/fd.hpp>
#include <scheduler/scheduler.hpp>

#include "test_helper.hpp"

#include <cstring>
#include <unistd.h>

TEST_CASE("StreamInputAdapter - receives data") {
    int fds[2];
    REQUIRE(pipe(fds) == 0);

    scheduler::Scheduler sched;
    io::FdConfig         config;
    config.fd                     = fds[0];
    config.owns_fd                = true;
    auto                   stream = std::make_shared<io::FdStream>("test", config);
    io::StreamInputAdapter adapter(stream);

    std::vector<uint8_t> received;
    adapter.callback = [&](io::Input&, uint8_t* data, size_t len) {
        received.insert(received.end(), data, data + len);
    };

    REQUIRE(adapter.schedule(sched));

    ::write(fds[1], "hello", 5);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return received.size() >= 5;
    }));

    CHECK(received.size() == 5);
    CHECK(memcmp(received.data(), "hello", 5) == 0);

    ::close(fds[1]);
    adapter.cancel();
}

TEST_CASE("StreamOutputAdapter - writes data") {
    int fds[2];
    REQUIRE(pipe(fds) == 0);

    scheduler::Scheduler sched;
    io::FdConfig         config;
    config.fd                      = fds[1];
    config.owns_fd                 = true;
    auto                    stream = std::make_shared<io::FdStream>("test", config);
    io::StreamOutputAdapter adapter(stream);

    REQUIRE(adapter.schedule(sched));

    adapter.write(reinterpret_cast<uint8_t const*>("world"), 5);
    sched.yield();
    sched.execute_once();

    char buf[10];
    auto n = ::read(fds[0], buf, sizeof(buf));
    CHECK(n == 5);
    CHECK(memcmp(buf, "world", 5) == 0);

    ::close(fds[0]);
    adapter.cancel();
}

TEST_CASE("StreamInputAdapter - on_complete called") {
    int fds[2];
    REQUIRE(pipe(fds) == 0);

    scheduler::Scheduler sched;
    io::FdConfig         config;
    config.fd                     = fds[0];
    config.owns_fd                = true;
    auto                   stream = std::make_shared<io::FdStream>("test", config);
    io::StreamInputAdapter adapter(stream);

    bool completed      = false;
    adapter.on_complete = [&]() {
        completed = true;
    };

    REQUIRE(adapter.schedule(sched));

    ::close(fds[1]);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return completed;
    }));
    CHECK(completed);

    adapter.cancel();
}

TEST_CASE("Multiple adapters share stream") {
    int fds[2];
    REQUIRE(pipe(fds) == 0);

    scheduler::Scheduler sched;
    io::FdConfig         config;
    config.fd      = fds[0];
    config.owns_fd = true;
    auto stream    = std::make_shared<io::FdStream>("test", config);

    io::StreamInputAdapter adapter1(stream);
    io::StreamInputAdapter adapter2(stream);

    int count1 = 0, count2 = 0;
    adapter1.callback = [&](io::Input&, uint8_t*, size_t) {
        count1++;
    };
    adapter2.callback = [&](io::Input&, uint8_t*, size_t) {
        count2++;
    };

    REQUIRE(adapter1.schedule(sched));
    REQUIRE(adapter2.schedule(sched));

    ::write(fds[1], "test", 4);
    REQUIRE(run_until_or_timeout(sched, [&] {
        return count1 > 0 && count2 > 0;
    }));

    CHECK(count1 == 1);
    CHECK(count2 == 1);

    ::close(fds[1]);
    adapter1.cancel();
    adapter2.cancel();
}
