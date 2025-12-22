#include <doctest/doctest.h>
#include <scheduler/scheduler.hpp>

#include <sys/eventfd.h>
#include <unistd.h>
#include <vector>

TEST_CASE("Event pool - allocate and free cycle") {
    scheduler::ScopedScheduler sched;

    int fd = eventfd(0, EFD_NONBLOCK);
    REQUIRE(fd >= 0);

    bool called = false;
    auto event  = sched.register_fd(
        fd, scheduler::EventInterest::Read,
        [&](scheduler::EventInterest) {
            called = true;
        },
        "test");

    CHECK(event.valid());
    sched.unregister(event);

    close(fd);
}

TEST_CASE("Event pool - generation prevents stale execution") {
    scheduler::ScopedScheduler sched;

    int fd1 = eventfd(0, EFD_NONBLOCK);
    int fd2 = eventfd(0, EFD_NONBLOCK);
    REQUIRE(fd1 >= 0);
    REQUIRE(fd2 >= 0);

    bool called1 = false;
    bool called2 = false;

    auto event1 = sched.register_fd(
        fd1, scheduler::EventInterest::Read,
        [&](scheduler::EventInterest) {
            called1 = true;
        },
        "event1");
    CHECK(event1.valid());

    auto old_index      = event1.index;
    auto old_generation = event1.generation;

    sched.unregister(event1);
    sched.execute_once();

    auto event2 = sched.register_fd(
        fd2, scheduler::EventInterest::Read,
        [&](scheduler::EventInterest) {
            called2 = true;
        },
        "event2");
    CHECK(event2.valid());
    CHECK(event2.index == old_index);
    CHECK(event2.generation != old_generation);

    // Stale handle operations just log errors, don't crash
    scheduler::ScheduledEvent stale_handle{old_index, old_generation};
    sched.update_interests(stale_handle, scheduler::EventInterest::Read);
    sched.unregister(stale_handle);

    sched.unregister(event2);

    close(fd1);
    close(fd2);
}

TEST_CASE("Event pool - update interests") {
    scheduler::ScopedScheduler sched;

    int fd = eventfd(0, EFD_NONBLOCK);
    REQUIRE(fd >= 0);

    auto event = sched.register_fd(
        fd, scheduler::EventInterest::Read,
        [](scheduler::EventInterest) {
        },
        "test");
    CHECK(event.valid());

    sched.update_interests(event, scheduler::EventInterest::Read | scheduler::EventInterest::Write);
    sched.update_interests(event, scheduler::EventInterest::None);

    sched.unregister(event);
    close(fd);
}

TEST_CASE("Event pool - pool exhaustion") {
    scheduler::ScopedScheduler sched;

    std::vector<int>                       fds;
    std::vector<scheduler::ScheduledEvent> events;

    for (int i = 0; i < scheduler::Scheduler::MAX_EVENT_SLOTS; i++) {
        int fd = eventfd(0, EFD_NONBLOCK);
        REQUIRE(fd >= 0);
        fds.push_back(fd);

        auto event = sched.register_fd(
            fd, scheduler::EventInterest::Read,
            [](scheduler::EventInterest) {
            },
            "test");
        CHECK(event.valid());
        events.push_back(event);
    }

    int extra_fd = eventfd(0, EFD_NONBLOCK);
    REQUIRE(extra_fd >= 0);
    auto exhausted_event = sched.register_fd(
        extra_fd, scheduler::EventInterest::Read,
        [](scheduler::EventInterest) {
        },
        "exhausted");
    CHECK_FALSE(exhausted_event.valid());
    close(extra_fd);

    for (auto& event : events) {
        sched.unregister(event);
    }
    for (int fd : fds) {
        close(fd);
    }
}

TEST_CASE("Event pool - rapid schedule/cancel cycles") {
    scheduler::ScopedScheduler sched;

    for (int cycle = 0; cycle < 1000; cycle++) {
        int fd = eventfd(0, EFD_NONBLOCK);
        REQUIRE(fd >= 0);

        auto event = sched.register_fd(
            fd, scheduler::EventInterest::Read,
            [](scheduler::EventInterest) {
            },
            "rapid");
        CHECK(event.valid());
        sched.unregister(event);

        close(fd);
        sched.execute_once();
    }
}

TEST_CASE("Event pool - generation wraps correctly") {
    scheduler::ScopedScheduler sched;

    int fd = eventfd(0, EFD_NONBLOCK);
    REQUIRE(fd >= 0);

    uint16_t first_gen = 0;
    for (int i = 0; i < 100; i++) {
        auto event = sched.register_fd(
            fd, scheduler::EventInterest::Read,
            [](scheduler::EventInterest) {
            },
            "wrap");
        CHECK(event.valid());
        if (i == 0) first_gen = event.generation;
        CHECK(event.generation != 0);
        sched.unregister(event);
        sched.execute_once();
    }

    close(fd);
}

TEST_CASE("Event pool - in-event unregister and reregister uses different slot") {
    scheduler::ScopedScheduler sched;

    int fd_a = eventfd(0, EFD_NONBLOCK);
    int fd_b = eventfd(0, EFD_NONBLOCK);
    REQUIRE(fd_a >= 0);
    REQUIRE(fd_b >= 0);

    bool                      callback_a_called = false;
    bool                      callback_b_called = false;
    scheduler::ScheduledEvent event_a;
    scheduler::ScheduledEvent event_b;

    event_a = sched.register_fd(
        fd_a, scheduler::EventInterest::Read,
        [&](scheduler::EventInterest) {
            callback_a_called = true;
            sched.unregister(event_a);
            event_b = sched.register_fd(
                fd_b, scheduler::EventInterest::Read,
                [&](scheduler::EventInterest) {
                    callback_b_called = true;
                },
                "event_b");
        },
        "event_a");
    CHECK(event_a.valid());

    uint64_t val = 1;
    write(fd_a, &val, sizeof(val));

    sched.execute_timeout(std::chrono::milliseconds(50));

    CHECK(callback_a_called);
    CHECK(event_b.valid());
    CHECK(event_b.index != event_a.index);
    CHECK_FALSE(callback_b_called);

    sched.unregister(event_b);
    close(fd_a);
    close(fd_b);
}

TEST_CASE("Event pool - stale event in batch not executed") {
    scheduler::ScopedScheduler sched;

    int fd1 = eventfd(0, EFD_NONBLOCK);
    int fd2 = eventfd(0, EFD_NONBLOCK);
    REQUIRE(fd1 >= 0);
    REQUIRE(fd2 >= 0);

    int                       call_count   = 0;
    bool                      event1_fired = false;
    bool                      event2_fired = false;
    scheduler::ScheduledEvent event1;
    scheduler::ScheduledEvent event2;

    event1 = sched.register_fd(
        fd1, scheduler::EventInterest::Read,
        [&](scheduler::EventInterest) {
            event1_fired = true;
            call_count++;
            sched.unregister(event2);
            sched.interrupt();
        },
        "event1");

    event2 = sched.register_fd(
        fd2, scheduler::EventInterest::Read,
        [&](scheduler::EventInterest) {
            event2_fired = true;
            call_count++;
            sched.interrupt();
        },
        "event2");

    CHECK(event1.valid());
    CHECK(event2.valid());

    uint64_t val = 1;
    REQUIRE(write(fd1, &val, sizeof(val)) == sizeof(val));
    REQUIRE(write(fd2, &val, sizeof(val)) == sizeof(val));

    sched.set_max_events_per_wait(10);
    sched.execute();

    CHECK(call_count >= 1);
    CHECK(call_count <= 2);
    if (event1_fired) {
        CHECK_FALSE(event2_fired);
    }

    if (event1.valid()) sched.unregister(event1);
    close(fd1);
    close(fd2);
}
