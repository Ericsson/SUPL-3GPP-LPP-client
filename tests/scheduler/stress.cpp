#include <cxx11_compat.hpp>
#include <doctest/doctest.h>
#include <memory>
#include <scheduler/scheduler.hpp>
#include <scheduler/socket.hpp>
#include <scheduler/timeout.hpp>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

TEST_CASE("Multiple concurrent timers") {
    scheduler::Scheduler sched;

    std::vector<std::unique_ptr<scheduler::TimeoutTask>> timers;
    std::vector<bool>                                    fired(10, false);

    for (int i = 0; i < 10; i++) {
        auto timeout =
            std::make_unique<scheduler::TimeoutTask>(std::chrono::milliseconds(10 + i * 5));
        timeout->callback = [&fired, i, &sched]() {
            fired[i] = true;
            if (i == 9) sched.interrupt();
        };
        timeout->schedule(sched);
        timers.push_back(std::move(timeout));
    }

    sched.execute();

    for (int i = 0; i < 10; i++) {
        CHECK(fired[i]);
    }
}

TEST_CASE("Many socket pairs") {
    scheduler::Scheduler sched;

    int const                                           N = 50;
    std::vector<int>                                    fds;
    std::vector<std::unique_ptr<scheduler::SocketTask>> tasks;
    std::vector<bool>                                   received(N, false);

    for (int i = 0; i < N; i++) {
        int pair[2];
        REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0);
        fds.push_back(pair[0]);
        fds.push_back(pair[1]);

        auto task     = std::make_unique<scheduler::SocketTask>(pair[0]);
        task->on_read = [&received, i, &sched](scheduler::SocketTask& t) {
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

        // Write to trigger read
        ::write(pair[1], "x", 1);
    }

    sched.execute_timeout(std::chrono::seconds(2));

    for (int i = 0; i < N; i++) {
        CHECK(received[i]);
    }

    for (auto fd : fds)
        ::close(fd);
}

TEST_CASE("Rapid timer creation and cancellation") {
    scheduler::Scheduler sched;

    int                                                  fired_count = 0;
    std::vector<std::unique_ptr<scheduler::TimeoutTask>> timers;

    for (int i = 0; i < 100; i++) {
        auto timeout = std::unique_ptr<scheduler::TimeoutTask>(
            new scheduler::TimeoutTask(std::chrono::milliseconds(10)));
        timeout->callback = [&fired_count]() {
            fired_count++;
        };
        timeout->schedule(sched);

        if (i % 2 == 0) {
            timeout->cancel();  // Cancel half
        } else {
            timers.push_back(std::move(timeout));  // Keep alive
        }
    }

    sched.execute_timeout(std::chrono::milliseconds(100));

    CHECK(fired_count <= 50);
    CHECK(fired_count > 0);
}

TEST_CASE("Deferred callback accumulation") {
    scheduler::Scheduler sched;

    int deferred_count = 0;

    scheduler::TimeoutTask timeout(std::chrono::milliseconds(10));
    timeout.callback = [&]() {
        for (int i = 0; i < 100; i++) {
            sched.defer([&deferred_count](scheduler::Scheduler&) {
                deferred_count++;
            });
        }
        sched.interrupt();
    };

    timeout.schedule(sched);
    sched.execute();

    CHECK(deferred_count == 100);
}

TEST_CASE("Deferred callback before execute") {
    scheduler::Scheduler sched;

    int deferred_count = 0;

    sched.defer([&deferred_count](scheduler::Scheduler&) {
        deferred_count++;
    });

    sched.execute();

    CHECK(deferred_count == 1);
}

TEST_CASE("Tick callback overhead") {
    scheduler::Scheduler sched;

    int              tick_count = 0;
    std::vector<int> dummy_data(100);

    for (int i = 0; i < 100; i++) {
        sched.register_tick(&dummy_data[i], [&tick_count]() {
            tick_count++;
        });
    }

    scheduler::TimeoutTask timeout(std::chrono::milliseconds(50));
    timeout.callback = [&sched]() {
        sched.interrupt();
    };
    timeout.schedule(sched);

    auto start = std::chrono::steady_clock::now();
    sched.execute();
    auto elapsed = std::chrono::steady_clock::now() - start;

    CHECK(tick_count >= 100);
    CHECK(elapsed < std::chrono::milliseconds(200));

    for (int i = 0; i < 100; i++) {
        sched.unregister_tick(&dummy_data[i]);
    }
}
