#include <cstring>
#include <doctest/doctest.h>
#include <fcntl.h>
#include <scheduler/scheduler.hpp>
#include <scheduler/stream.hpp>
#include <unistd.h>

TEST_CASE("StreamTask basic write") {
    scheduler::Scheduler  sched;
    scheduler::StreamTask stream(1024, std::chrono::milliseconds(10));

    int write_count = 0;
    stream.callback = [&](int fd, size_t block_size) {
        char const* msg = "test";
        ::write(fd, msg, strlen(msg));
        write_count++;
        if (write_count >= 3) sched.interrupt();
    };

    CHECK(stream.schedule(sched));
    sched.execute();

    CHECK(write_count == 3);

    // Read from stream
    char buf[256];
    auto n = ::read(stream.fd(), buf, sizeof(buf));
    CHECK(n == 12);  // "test" × 3
}

TEST_CASE("ForwardStreamTask file to pipe") {
    scheduler::Scheduler sched;

    // Create temp file
    char const* path    = "/tmp/test_forward.txt";
    int         file_fd = ::open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    REQUIRE(file_fd >= 0);

    char const* content = "hello world\n";
    ::write(file_fd, content, strlen(content));
    ::close(file_fd);

    // Open for reading
    file_fd = ::open(path, O_RDONLY);
    REQUIRE(file_fd >= 0);

    scheduler::ForwardStreamTask forward(file_fd, 1024, std::chrono::milliseconds(10));

    bool completed      = false;
    forward.on_complete = [&]() {
        completed = true;
        sched.interrupt();
    };

    CHECK(forward.schedule(sched));
    sched.execute_timeout(std::chrono::milliseconds(500));

    CHECK(completed);

    // Read forwarded data
    char buf[256];
    auto n = ::read(forward.fd(), buf, sizeof(buf));
    CHECK(n == strlen(content));
    CHECK(std::string(buf, n) == content);

    ::close(file_fd);
    ::unlink(path);
}

TEST_CASE("StreamTask rate limiting") {
    scheduler::Scheduler  sched;
    scheduler::StreamTask stream(1024, std::chrono::milliseconds(50));

    std::vector<std::chrono::steady_clock::time_point> timestamps;

    stream.callback = [&](int fd, size_t) {
        timestamps.push_back(std::chrono::steady_clock::now());
        ::write(fd, "x", 1);
        if (timestamps.size() >= 3) sched.interrupt();
    };

    stream.schedule(sched);
    sched.execute();

    REQUIRE(timestamps.size() == 3);

    auto delta1 = timestamps[1] - timestamps[0];
    auto delta2 = timestamps[2] - timestamps[1];

    CHECK(delta1 >= std::chrono::milliseconds(40));
    CHECK(delta2 >= std::chrono::milliseconds(40));
}
