#include <cxx11_compat.hpp>
#include <doctest/doctest.h>
#include <memory>
#include <scheduler/file_descriptor.hpp>
#include <scheduler/scheduler.hpp>
#include <scheduler/timeout.hpp>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

TEST_CASE("Multiple concurrent timers") {
    scheduler::ScopedScheduler sched;

    std::vector<std::unique_ptr<scheduler::TimeoutTask>> timers;
    std::vector<bool>                                    fired(10, false);

    for (int i = 0; i < 10; i++) {
        auto timeout = std::make_unique<scheduler::TimeoutTask>(
            std::chrono::milliseconds(10 + i * 5), [&fired, i, &sched]() {
                fired[i] = true;
                if (i == 9) sched.interrupt();
            });
        timers.push_back(std::move(timeout));
    }

    sched.execute();

    for (int i = 0; i < 10; i++) {
        CHECK(fired[i]);
    }
}

TEST_CASE("Many socket pairs") {
    scheduler::ScopedScheduler sched;

    int const                                                        N = 50;
    std::vector<int>                                                 write_fds;
    std::vector<std::unique_ptr<scheduler::OwnedFileDescriptorTask>> tasks;
    std::vector<bool>                                                received(N, false);

    for (int i = 0; i < N; i++) {
        int pair[2];
        REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0);
        write_fds.push_back(pair[1]);

        auto task     = std::make_unique<scheduler::OwnedFileDescriptorTask>(pair[0]);
        task->on_read = [&received, i, &sched](scheduler::OwnedFileDescriptorTask& t) {
            char buf[1];
            ::read(t.fd(), buf, 1);
            received[i] = true;

            bool all_done = true;
            for (auto r : received) {
                if (!r) {
                    all_done = false;
                    break;
                }
            }
            if (all_done) sched.interrupt();
        };
        task->schedule(sched);
        tasks.push_back(std::move(task));

        ::write(pair[1], "x", 1);
    }

    sched.execute_timeout(std::chrono::seconds(1));

    for (int i = 0; i < N; i++) {
        CHECK(received[i]);
    }

    for (auto fd : write_fds)
        ::close(fd);
}

TEST_CASE("Rapid timer creation and cancellation") {
    scheduler::ScopedScheduler sched;

    int                                                  fired_count = 0;
    std::vector<std::unique_ptr<scheduler::TimeoutTask>> timers;

    for (int i = 0; i < 100; i++) {
        auto timeout = std::make_unique<scheduler::TimeoutTask>(std::chrono::milliseconds(10),
                                                                [&fired_count]() {
                                                                    fired_count++;
                                                                });

        if (i % 2 == 0) {
            timeout->cancel();
        } else {
            timers.push_back(std::move(timeout));
        }
    }

    sched.execute_timeout(std::chrono::milliseconds(100));

    CHECK(fired_count <= 50);
    CHECK(fired_count > 0);
}

TEST_CASE("Deferred callback accumulation") {
    scheduler::ScopedScheduler sched;

    int deferred_count = 0;

    scheduler::TimeoutTask timeout(std::chrono::milliseconds(10), [&]() {
        for (int i = 0; i < 100; i++) {
            sched.defer([&deferred_count](scheduler::Scheduler&) {
                deferred_count++;
            });
        }
        sched.interrupt();
    });

    sched.execute();

    CHECK(deferred_count == 100);
}

TEST_CASE("Deferred callback before execute") {
    scheduler::ScopedScheduler sched;

    int deferred_count = 0;

    sched.defer([&deferred_count](scheduler::Scheduler&) {
        deferred_count++;
    });

    sched.execute();

    CHECK(deferred_count == 1);
}
