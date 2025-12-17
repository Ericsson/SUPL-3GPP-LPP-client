#include <doctest/doctest.h>
#include <io/write_buffer.hpp>

TEST_CASE("WriteBuffer - enqueue and peek") {
    io::WriteBuffer buf(1024);

    uint8_t data[] = {1, 2, 3, 4, 5};
    buf.enqueue(data, 5);
    CHECK(buf.size() == 5);
    CHECK_FALSE(buf.empty());

    auto [ptr, len] = buf.peek();
    CHECK(len == 5);
    CHECK(ptr[0] == 1);
    CHECK(ptr[4] == 5);
}

TEST_CASE("WriteBuffer - consume") {
    io::WriteBuffer buf(1024);

    uint8_t data[] = {1, 2, 3, 4, 5};
    buf.enqueue(data, 5);

    buf.consume(3);
    CHECK(buf.size() == 2);

    auto [ptr, len] = buf.peek();
    CHECK(len == 2);
    CHECK(ptr[0] == 4);
    CHECK(ptr[1] == 5);
}

TEST_CASE("WriteBuffer - consume all clears buffer") {
    io::WriteBuffer buf(1024);

    uint8_t data[] = {1, 2, 3};
    buf.enqueue(data, 3);
    buf.consume(3);

    CHECK(buf.empty());
    CHECK(buf.size() == 0);

    auto [ptr, len] = buf.peek();
    CHECK(ptr == nullptr);
    CHECK(len == 0);
}

TEST_CASE("WriteBuffer - full buffer discards old data") {
    io::WriteBuffer buf(10);

    uint8_t data1[] = {1, 2, 3, 4, 5, 6, 7, 8};
    buf.enqueue(data1, 8);
    CHECK(buf.size() == 8);

    uint8_t data2[] = {10, 11, 12, 13, 14};
    buf.enqueue(data2, 5);
    CHECK(buf.size() == 10);

    auto [ptr, len] = buf.peek();
    CHECK(len == 10);
    CHECK(ptr[0] == 4);  // Old data discarded from front
    CHECK(ptr[4] == 8);
    CHECK(ptr[5] == 10);  // New data at end
    CHECK(ptr[9] == 14);
}

TEST_CASE("WriteBuffer - data larger than max truncates") {
    io::WriteBuffer buf(5);

    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    buf.enqueue(data, 10);
    CHECK(buf.size() == 5);

    auto [ptr, len] = buf.peek();
    CHECK(len == 5);
    CHECK(ptr[0] == 6);  // Only tail kept
    CHECK(ptr[4] == 10);
}

TEST_CASE("WriteBuffer - clear") {
    io::WriteBuffer buf(1024);

    uint8_t data[] = {1, 2, 3};
    buf.enqueue(data, 3);
    buf.clear();

    CHECK(buf.empty());
    CHECK(buf.size() == 0);
}

TEST_CASE("WriteBuffer - multiple enqueue") {
    io::WriteBuffer buf(1024);

    uint8_t data1[] = {1, 2};
    uint8_t data2[] = {3, 4, 5};
    buf.enqueue(data1, 2);
    buf.enqueue(data2, 3);

    CHECK(buf.size() == 5);

    auto [ptr, len] = buf.peek();
    CHECK(len == 5);
    CHECK(ptr[0] == 1);
    CHECK(ptr[2] == 3);
}

TEST_CASE("WriteBuffer - partial consume then enqueue") {
    io::WriteBuffer buf(1024);

    uint8_t data1[] = {1, 2, 3, 4};
    buf.enqueue(data1, 4);
    buf.consume(2);

    uint8_t data2[] = {5, 6};
    buf.enqueue(data2, 2);

    CHECK(buf.size() == 4);
}
