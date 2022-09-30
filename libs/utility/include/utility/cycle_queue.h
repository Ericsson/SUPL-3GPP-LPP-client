#pragma once
#include "utility/types.h"
#include "utility/mutex.h"
#include "utility/sem.h"

template <typename T, s64 C>
class Queue {
public:
    Queue() : available(0) {
        write = 0;
        read  = 0;
        len   = 0;
    }

    NO_DISCARD auto push(T&& value) {
        struct Result {
            bool success;
            T    overwritten_value;
        };

        MutexScope scope{mutex};

        T    overwritten{};
        auto next_write = (write + 1) % C;
        if (next_write == read) {
            overwritten = data[next_write];
            // overwrite value
            auto next_read = (read + 1) % C;
            read           = next_read;
            len--;
            available.wait();  // should never wait
        }

        data[write] = std::move(value);
        write       = next_write;
        len++;
        available.post();
        return Result{true, overwritten};
    }

    NO_DISCARD auto pop(timespec timeout) {
        struct Result {
            bool success;
            T    value;
        };

        if (!available.wait_timeout(timeout)) {
            return Result{false, {}};
        }

        MutexScope scope{mutex};

        auto next_read = (read + 1) % C;
        assert(read != write);

        auto& value = data[read];
        read        = next_read;
        len--;
        return Result{true, std::move(value)};
    }

    auto length() const { return len; }

private:
    Mutex            mutex;
    Semaphore        available;
    s64              write;
    s64              read;
    s64              len;
    std::array<T, C> data;
};