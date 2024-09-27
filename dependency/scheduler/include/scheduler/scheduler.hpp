#pragma once
#include <core/core.hpp>

#include <chrono>
#include <functional>
#include <sys/epoll.h>

namespace scheduler {
struct EpollEvent {
    std::function<void(struct epoll_event*)> event;
};

class Scheduler {
public:
    Scheduler() NOEXCEPT;
    ~Scheduler() NOEXCEPT;

    void execute() NOEXCEPT;
    void execute_timeout(std::chrono::steady_clock::duration duration) NOEXCEPT;
    void execute_while(std::function<bool()> condition) NOEXCEPT;

    // Add, remove, and update file descriptors in the epoll instance.
    NODISCARD bool add_epoll_fd(int fd, uint32_t events,
                                          EpollEvent* event) NOEXCEPT;
    NODISCARD bool update_epoll_fd(int fd, uint32_t events,
                                             EpollEvent* event) NOEXCEPT;
    NODISCARD bool remove_epoll_fd(int fd) NOEXCEPT;

private:
    void process_event(struct epoll_event& event) NOEXCEPT;

    int mEpollFd;
    int mInterruptFd;
    int mEpollCount;
};
}  // namespace scheduler
