#include <doctest/doctest.h>
#include <io/registry.hpp>
#include <io/stream/fd.hpp>
#include <scheduler/scheduler.hpp>

#include <unistd.h>

TEST_CASE("StreamRegistry - add and get") {
    io::StreamRegistry registry;

    int fds[2];
    REQUIRE(pipe(fds) == 0);

    io::FdConfig config;
    config.fd      = fds[0];
    config.owns_fd = true;
    auto stream    = std::make_shared<io::FdStream>("test-stream", config);
    registry.add("test", stream);

    CHECK(registry.has("test"));
    CHECK(registry.get("test") == stream);
    CHECK_FALSE(registry.has("nonexistent"));
    CHECK(registry.get("nonexistent") == nullptr);

    ::close(fds[1]);
}

TEST_CASE("StreamRegistry - schedule_all") {
    io::StreamRegistry   registry;
    scheduler::Scheduler sched;

    int fds1[2], fds2[2];
    REQUIRE(pipe(fds1) == 0);
    REQUIRE(pipe(fds2) == 0);

    io::FdConfig config1;
    config1.fd      = fds1[0];
    config1.owns_fd = true;
    io::FdConfig config2;
    config2.fd      = fds2[0];
    config2.owns_fd = true;

    auto stream1 = std::make_shared<io::FdStream>("stream1", config1);
    auto stream2 = std::make_shared<io::FdStream>("stream2", config2);

    registry.add("s1", stream1);
    registry.add("s2", stream2);

    CHECK(registry.schedule_all(sched));
    CHECK(stream1->state() == io::Stream::State::Connected);
    CHECK(stream2->state() == io::Stream::State::Connected);

    registry.cancel_all();

    ::close(fds1[1]);
    ::close(fds2[1]);
}

TEST_CASE("StreamRegistry - schedule_all skips already scheduled") {
    io::StreamRegistry   registry;
    scheduler::Scheduler sched;

    int fds[2];
    REQUIRE(pipe(fds) == 0);

    io::FdConfig config;
    config.fd      = fds[0];
    config.owns_fd = true;
    auto stream    = std::make_shared<io::FdStream>("test", config);
    registry.add("test", stream);

    REQUIRE(stream->schedule(sched));
    CHECK(stream->state() == io::Stream::State::Connected);

    CHECK(registry.schedule_all(sched));

    registry.cancel_all();
    ::close(fds[1]);
}
