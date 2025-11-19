#include <doctest/doctest.h>
#include <io/buffer.hpp>
#include <scheduler/scheduler.hpp>

TEST_CASE("BufferInput - basic read") {
    io::BufferInput      input;
    scheduler::Scheduler scheduler;

    bool     received = false;
    uint8_t* data_ptr = nullptr;
    size_t   data_len = 0;

    input.callback = [&](io::Input&, uint8_t* data, size_t len) {
        received = true;
        data_ptr = data;
        data_len = len;
    };

    input.schedule(scheduler);
    input.push(reinterpret_cast<uint8_t const*>("test"), 4);

    scheduler.execute_while([&]() {
        return !received;
    });

    CHECK(received);
    CHECK(data_len == 4);
}

TEST_CASE("BufferOutput - basic write") {
    io::BufferOutput output;

    output.write(reinterpret_cast<uint8_t const*>("hello"), 5);

    CHECK(output.data().size() == 5);
    CHECK(output.data()[0] == 'h');
    CHECK(output.data()[4] == 'o');
}
