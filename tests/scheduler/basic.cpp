#include <chrono>
#include <doctest/doctest.h>
#include <scheduler/file_descriptor.hpp>
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>
#include <scheduler/timeout.hpp>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

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
    scheduler::ScopedScheduler sched;

    bool                   fired = false;
    scheduler::TimeoutTask timeout(std::chrono::milliseconds(50), [&]() {
        fired = true;
    });

    sched.execute_timeout(std::chrono::milliseconds(200));

    CHECK(fired);
}

TEST_CASE("TimeoutTask cancel") {
    scheduler::ScopedScheduler sched;

    bool                   fired = false;
    scheduler::TimeoutTask timeout(std::chrono::milliseconds(50), [&]() {
        fired = true;
    });

    timeout.cancel();

    sched.execute_timeout(std::chrono::milliseconds(100));
    CHECK_FALSE(fired);
}

TEST_CASE("PeriodicTask fires multiple times") {
    scheduler::ScopedScheduler sched;
    scheduler::PeriodicTask    periodic(std::chrono::milliseconds(20));

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
    scheduler::ScopedScheduler sched;

    int order          = 0;
    int event_order    = 0;
    int deferred_order = 0;

    scheduler::TimeoutTask timeout(std::chrono::milliseconds(10), [&]() {
        event_order = ++order;
        sched.defer([&](scheduler::Scheduler&) {
            deferred_order = ++order;
        });
        sched.interrupt();
    });

    sched.execute();

    CHECK(event_order == 1);
    CHECK(deferred_order == 2);
}

TEST_CASE("TimeoutTask move") {
    scheduler::ScopedScheduler sched;

    bool fired    = false;
    auto timeout1 = std::make_unique<scheduler::TimeoutTask>(std::chrono::milliseconds(50), [&]() {
        fired = true;
    });

    CHECK(timeout1->is_scheduled());

    // Move construct
    scheduler::TimeoutTask timeout2(std::move(*timeout1));
    CHECK_FALSE(timeout1->is_scheduled());
    CHECK(timeout2.is_scheduled());

    sched.execute_timeout(std::chrono::milliseconds(200));
    CHECK(fired);
}

TEST_CASE("FileDescriptorTask move") {
    scheduler::ScopedScheduler sched;

    int fds[2];
    REQUIRE(pipe(fds) == 0);

    bool read_called = false;

    scheduler::FileDescriptorTask task1;
    task1.set_fd(fds[0]);
    task1.on_read = [&](scheduler::FileDescriptorTask&) {
        read_called = true;
        sched.interrupt();
    };
    task1.schedule(sched);

    CHECK(task1.is_scheduled());
    CHECK(task1.fd() == fds[0]);

    // Move construct
    scheduler::FileDescriptorTask task2(std::move(task1));
    CHECK_FALSE(task1.is_scheduled());
    CHECK(task1.fd() == -1);
    CHECK(task2.is_scheduled());
    CHECK(task2.fd() == fds[0]);

    // Trigger read
    ::write(fds[1], "x", 1);
    sched.execute_timeout(std::chrono::milliseconds(100));

    CHECK(read_called);

    ::close(fds[0]);
    ::close(fds[1]);
}

TEST_CASE("OwnedFileDescriptorTask move") {
    scheduler::ScopedScheduler sched;

    int fds[2];
    REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    bool read_called = false;

    auto task1     = std::make_unique<scheduler::OwnedFileDescriptorTask>(fds[0]);
    task1->on_read = [&](scheduler::OwnedFileDescriptorTask&) {
        read_called = true;
        sched.interrupt();
    };
    task1->schedule(sched);

    CHECK(task1->is_scheduled());
    CHECK(task1->fd() == fds[0]);

    // Move construct
    scheduler::OwnedFileDescriptorTask task2(std::move(*task1));
    CHECK_FALSE(task1->is_scheduled());
    CHECK(task1->fd() == -1);
    CHECK(task2.is_scheduled());
    CHECK(task2.fd() == fds[0]);

    // Trigger read
    ::write(fds[1], "x", 1);
    sched.execute_timeout(std::chrono::milliseconds(100));

    CHECK(read_called);

    ::close(fds[1]);
    // fds[0] owned by task2, will be closed in destructor
}

TEST_CASE("RepeatableTimeoutTask move") {
    scheduler::ScopedScheduler sched;

    bool fired = false;

    scheduler::RepeatableTimeoutTask task1(std::chrono::milliseconds(50));
    task1.callback = [&]() {
        fired = true;
        sched.interrupt();
    };
    task1.schedule();

    CHECK(task1.is_scheduled());

    // Move construct
    scheduler::RepeatableTimeoutTask task2(std::move(task1));
    CHECK_FALSE(task1.is_scheduled());
    CHECK(task2.is_scheduled());

    sched.execute_timeout(std::chrono::milliseconds(200));
    CHECK(fired);
}
