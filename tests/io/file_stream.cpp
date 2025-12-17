#include <doctest/doctest.h>
#include <io/stream/file.hpp>
#include <scheduler/scheduler.hpp>

#include "test_helper.hpp"

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

TEST_CASE("FileStream - write to regular file") {
    LogletTesting        loglet;
    scheduler::Scheduler scheduler;

    char temp_path[] = "/tmp/file_stream_test_XXXXXX";
    int  temp_fd     = mkstemp(temp_path);
    REQUIRE(temp_fd >= 0);
    close(temp_fd);

    io::FileConfig config;
    config.path     = temp_path;
    config.read     = false;
    config.write    = true;
    config.truncate = true;
    io::FileStream stream("writer", config);
    REQUIRE(stream.schedule(scheduler));

    uint8_t const data[] = "test file content";
    stream.write(data, sizeof(data));

    stream.cancel();

    FILE* f = fopen(temp_path, "r");
    REQUIRE(f != nullptr);
    char   buf[64] = {};
    size_t n       = fread(buf, 1, sizeof(buf), f);
    fclose(f);

    CHECK(n == sizeof(data));
    CHECK(memcmp(buf, data, sizeof(data)) == 0);

    unlink(temp_path);
}

TEST_CASE("FileStream - read from FIFO") {
    LogletTesting        loglet;
    scheduler::Scheduler scheduler;

    char temp_dir[] = "/tmp/file_stream_XXXXXX";
    REQUIRE(mkdtemp(temp_dir) != nullptr);
    std::string fifo_path = std::string(temp_dir) + "/fifo";
    REQUIRE(mkfifo(fifo_path.c_str(), 0666) == 0);

    int write_fd = open(fifo_path.c_str(), O_WRONLY | O_NONBLOCK);
    if (write_fd < 0) {
        unlink(fifo_path.c_str());
        rmdir(temp_dir);
        return;
    }

    io::FileConfig config;
    config.path = fifo_path;
    config.read = true;
    io::FileStream stream("reader", config);

    std::vector<uint8_t> recv;
    stream.on_read([&](io::Stream&, uint8_t* d, size_t len) {
        recv.insert(recv.end(), d, d + len);
    });

    REQUIRE(stream.schedule(scheduler));

    uint8_t const data[]  = "fifo test data";
    ssize_t       written = ::write(write_fd, data, sizeof(data));
    REQUIRE(written == sizeof(data));

    run_until_or_timeout(
        scheduler,
        [&] {
            return recv.size() >= sizeof(data);
        },
        std::chrono::milliseconds(2000));

    CHECK(recv.size() == sizeof(data));
    CHECK(memcmp(recv.data(), data, sizeof(data)) == 0);

    close(write_fd);
    unlink(fifo_path.c_str());
    rmdir(temp_dir);
}

TEST_CASE("FileStream - rate-limited read") {
    LogletTesting        loglet;
    scheduler::Scheduler scheduler;

    char temp_path[] = "/tmp/file_stream_rate_XXXXXX";
    int  temp_fd     = mkstemp(temp_path);
    REQUIRE(temp_fd >= 0);

    uint8_t data[100];
    for (size_t i = 0; i < sizeof(data); i++)
        data[i] = static_cast<uint8_t>(i);
    REQUIRE(::write(temp_fd, data, sizeof(data)) == sizeof(data));
    close(temp_fd);

    io::FileConfig config;
    config.path           = temp_path;
    config.read           = true;
    config.bytes_per_tick = 10;
    config.tick_interval  = std::chrono::milliseconds(50);
    io::FileStream stream("rate-reader", config);

    std::vector<uint8_t> recv;
    size_t               callback_count = 0;
    stream.on_read([&](io::Stream&, uint8_t* d, size_t len) {
        recv.insert(recv.end(), d, d + len);
        callback_count++;
    });

    REQUIRE(stream.schedule(scheduler));

    auto start = std::chrono::steady_clock::now();
    run_until_or_timeout(
        scheduler,
        [&] {
            return recv.size() >= sizeof(data);
        },
        std::chrono::milliseconds(2000));
    auto elapsed = std::chrono::steady_clock::now() - start;

    CHECK(recv.size() == sizeof(data));
    CHECK(memcmp(recv.data(), data, sizeof(data)) == 0);
    CHECK(callback_count >= 10);
    CHECK(elapsed >= std::chrono::milliseconds(400));

    stream.cancel();
    unlink(temp_path);
}

TEST_CASE("FileStream - read buffering with min_bytes") {
    LogletTesting        loglet;
    scheduler::Scheduler scheduler;

    char temp_dir[] = "/tmp/file_stream_buf_XXXXXX";
    REQUIRE(mkdtemp(temp_dir) != nullptr);
    std::string fifo_path = std::string(temp_dir) + "/fifo";
    REQUIRE(mkfifo(fifo_path.c_str(), 0666) == 0);

    int write_fd = open(fifo_path.c_str(), O_WRONLY | O_NONBLOCK);
    if (write_fd < 0) {
        unlink(fifo_path.c_str());
        rmdir(temp_dir);
        return;
    }

    io::FileConfig config;
    config.path                  = fifo_path;
    config.read                  = true;
    config.read_config.min_bytes = 20;
    io::FileStream stream("buf-reader", config);

    std::vector<size_t> recv_sizes;
    stream.on_read([&](io::Stream&, uint8_t*, size_t len) {
        recv_sizes.push_back(len);
    });

    REQUIRE(stream.schedule(scheduler));

    for (int i = 0; i < 5; i++) {
        uint8_t chunk[10] = {};
        ::write(write_fd, chunk, sizeof(chunk));
    }

    run_until_or_timeout(
        scheduler,
        [&] {
            return recv_sizes.size() >= 2;
        },
        std::chrono::milliseconds(500));

    CHECK(recv_sizes.size() >= 2);
    for (auto sz : recv_sizes) {
        CHECK(sz >= 20);
    }

    close(write_fd);
    unlink(fifo_path.c_str());
    rmdir(temp_dir);
}

TEST_CASE("FileStream - read buffering with timeout flush") {
    LogletTesting        loglet;
    scheduler::Scheduler scheduler;

    char temp_dir[] = "/tmp/file_stream_flush_XXXXXX";
    REQUIRE(mkdtemp(temp_dir) != nullptr);
    std::string fifo_path = std::string(temp_dir) + "/fifo";
    REQUIRE(mkfifo(fifo_path.c_str(), 0666) == 0);

    int write_fd = open(fifo_path.c_str(), O_WRONLY | O_NONBLOCK);
    if (write_fd < 0) {
        unlink(fifo_path.c_str());
        rmdir(temp_dir);
        return;
    }

    io::FileConfig config;
    config.path                  = fifo_path;
    config.read                  = true;
    config.read_config.min_bytes = 100;
    config.read_config.timeout   = std::chrono::milliseconds(100);
    io::FileStream stream("flush-reader", config);

    std::vector<uint8_t> recv;
    stream.on_read([&](io::Stream&, uint8_t* d, size_t len) {
        recv.insert(recv.end(), d, d + len);
    });

    REQUIRE(stream.schedule(scheduler));

    uint8_t small_data[5] = {1, 2, 3, 4, 5};
    ::write(write_fd, small_data, sizeof(small_data));

    run_until_or_timeout(
        scheduler,
        [&] {
            return recv.size() >= sizeof(small_data);
        },
        std::chrono::milliseconds(500));

    CHECK(recv.size() == sizeof(small_data));

    close(write_fd);
    unlink(fifo_path.c_str());
    rmdir(temp_dir);
}
