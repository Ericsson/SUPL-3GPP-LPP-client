#include "scheduler/task.hpp"
#include "scheduler/scheduler.hpp"

#include <stdio.h>

IoTask::IoTask(int fd) : mFd(fd) {
    mEvent = {[this](struct epoll_event* event) {
        this->event(event);
    }};
}

void IoTask::register_task(Scheduler* scheduler) {
    Task::register_task(scheduler);
    auto events = 0;
    if (on_read) {
        events |= EPOLLIN;
    }
    if (on_write) {
        events |= EPOLLOUT;
    }
    if (on_error) {
        events |= EPOLLERR;
    }
    mScheduler->add_epoll_fd(mFd, events, &mEvent);
}

void IoTask::unregister_task(Scheduler* scheduler) {
    Task::unregister_task(scheduler);
    mScheduler->remove_epoll_fd(mFd);
}

void IoTask::event(struct epoll_event* event) {
    if (event->events & EPOLLIN) {
        if (on_read) {
            on_read(mFd);
        }
    }
    if (event->events & EPOLLOUT) {
        if (on_write) {
            on_write(mFd);
        }
    }
    if (event->events & EPOLLERR) {
        if (on_error) {
            on_error(mFd);
        }
    }
}