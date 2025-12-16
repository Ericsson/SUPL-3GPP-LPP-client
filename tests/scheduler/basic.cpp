#include <chrono>
#include <doctest/doctest.h>
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>
#include <scheduler/timeout.hpp>
#include <thread>

TEST_CASE("Scheduler creation and destruction") {
    scheduler::Scheduler sched;
}

TEST_CASE("Scheduler execute_timeout with no events") {
    scheduler::Scheduler sched;
    auto                 start = std::chrono::steady_clock::now();
    sched.execute_timeout(std::chrono::milliseconds(100));
    auto elapsed = std::chrono::steady_clock::now() - start;

    CHECK(elapsed >= std::chrono::milliseconds(90));
    CHECK(elapsed <= std::chrono::milliseconds(150));
}

TEST_CASE("Scheduler interrupt") {
    scheduler::Scheduler sched;

    bool        interrupted = false;
    std::thread t([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sched.interrupt();
        interrupted = true;
    });

    auto start = std::chrono::steady_clock::now();
    sched.execute_timeout(std::chrono::seconds(10));
    auto elapsed = std::chrono::steady_clock::now() - start;

    t.join();
    CHECK(interrupted);
    CHECK(elapsed < std::chrono::milliseconds(200));
}

TEST_CASE("TimeoutTask basic") {
    scheduler::Scheduler   sched;
    scheduler::TimeoutTask timeout(std::chrono::milliseconds(50));

    bool fired       = false;
    timeout.callback = [&]() {
        fired = true;
    };

    CHECK(timeout.schedule(sched));
    sched.execute_timeout(std::chrono::milliseconds(200));

    CHECK(fired);
}

TEST_CASE("TimeoutTask cancel") {
    scheduler::Scheduler   sched;
    scheduler::TimeoutTask timeout(std::chrono::milliseconds(50));

    bool fired       = false;
    timeout.callback = [&]() {
        fired = true;
    };

    CHECK(timeout.schedule(sched));
    CHECK(timeout.cancel());

    sched.execute_timeout(std::chrono::milliseconds(100));
    CHECK_FALSE(fired);
}

TEST_CASE("PeriodicTask fires multiple times") {
    scheduler::Scheduler    sched;
    scheduler::PeriodicTask periodic(std::chrono::milliseconds(20));

    int count         = 0;
    periodic.callback = [&]() {
        count++;
        if (count >= 3) sched.interrupt();
    };

    CHECK(periodic.schedule(sched));
    sched.execute();

    CHECK(count == 3);
}

TEST_CASE("Deferred callbacks execute after events") {
    scheduler::Scheduler sched;

    int order          = 0;
    int event_order    = 0;
    int deferred_order = 0;

    scheduler::TimeoutTask timeout(std::chrono::milliseconds(10));
    timeout.callback = [&]() {
        event_order = ++order;
        sched.defer([&](scheduler::Scheduler&) {
            deferred_order = ++order;
        });
        sched.interrupt();
    };

    timeout.schedule(sched);
    sched.execute();

    CHECK(event_order == 1);
    CHECK(deferred_order == 2);
}

TEST_CASE("Tick callbacks run every iteration") {
    scheduler::Scheduler sched;

    int tick_count  = 0;
    int event_count = 0;

    sched.register_tick(&tick_count, [&]() {
        tick_count++;
    });

    scheduler::PeriodicTask periodic(std::chrono::milliseconds(10));
    periodic.callback = [&]() {
        event_count++;
        if (event_count >= 3) sched.interrupt();
    };

    periodic.schedule(sched);
    sched.execute();

    CHECK(event_count == 3);
    CHECK(tick_count >= event_count);

    sched.unregister_tick(&tick_count);
}
