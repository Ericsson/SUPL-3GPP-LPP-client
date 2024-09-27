#include "scheduler/task.hpp"
#include "scheduler/scheduler.hpp"

#include <stdio.h>

IoTask::IoTask(int fd) : mFd(fd), mActive(false) {
    mEvent = {[this](struct epoll_event* event) {
        this->event(event);
    }};
}

void IoTask::register_task(Scheduler* scheduler) {
    Task::register_task(scheduler);
    if(!mActive) {
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
        mActive = mScheduler->add_epoll_fd(mFd, events, &mEvent);
    }
}

void IoTask::unregister_task(Scheduler* scheduler) {
    Task::unregister_task(scheduler);
    if(mActive) {
        mScheduler->remove_epoll_fd(mFd);
        mActive = false;
    }
}

void IoTask::event(struct epoll_event* event) {
    if(mActive) {
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
    }}
}