#include "file_descriptor.hpp"

#include <errno.h>
#include <fcntl.h>
#include <functional>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(sched, task);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(sched, task)

namespace scheduler {
FileDescriptorTask::FileDescriptorTask() NOEXCEPT : on_read{},
                                                    on_error{},
                                                    on_write{},
                                                    mScheduler{nullptr},
                                                    mEvent{},
                                                    mFd{-1} {
    VSCOPE_FUNCTION();
    mEvent.name  = "fd";
    mEvent.event = [this](struct epoll_event* event) {
        VERBOSEF("fd task: event: %d %s%s%s%s", mFd, (event->events & EPOLLIN) ? "read " : "",
                 (event->events & EPOLLOUT) ? "write " : "",
                 (event->events & EPOLLERR) ? "error " : "",
                 (event->events & EPOLLHUP) ? "hup " : "");
        if ((event->events & EPOLLIN) != 0 && this->on_read) {
            TRACE_INDENT_SCOPE();
            this->on_read(mFd);
        }

        if ((event->events & EPOLLOUT) != 0 && this->on_write) {
            TRACE_INDENT_SCOPE();
            this->on_write(mFd);
        }

        if ((event->events & (EPOLLERR | EPOLLHUP)) != 0 && this->on_error) {
            TRACE_INDENT_SCOPE();
            this->on_error(mFd);
        }
    };
}

FileDescriptorTask::~FileDescriptorTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mScheduler) {
        cancel();
    }

    mFd = -1;
}

bool FileDescriptorTask::schedule(Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    if (mScheduler) {
        WARNF("already scheduled");
        return false;
    } else if (mFd < 0) {
        WARNF("invalid file descriptor");
        return false;
    }

    uint32_t events = 0;
    if (on_read) events |= EPOLLIN;
    if (on_write) events |= EPOLLOUT;
    if (on_error) events |= EPOLLERR;
    if (scheduler.add_epoll_fd(mFd, events, &mEvent)) {
        mScheduler = &scheduler;
        return true;
    } else {
        return false;
    }
}

bool FileDescriptorTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mScheduler) {
        WARNF("not scheduled");
        return false;
    }

    if (mScheduler->remove_epoll_fd(mFd)) {
        mScheduler = nullptr;
        return true;
    } else {
        return false;
    }
}

bool FileDescriptorTask::set_fd(int fd) NOEXCEPT {
    VSCOPE_FUNCTIONF("%d", fd);
    if (mScheduler) {
        VERBOSEF("fd task: cancelling existing task due to new file descriptor");
        cancel();
        mFd = fd;
        return true;
    } else {
        mFd = fd;
        return false;
    }
}
}  // namespace scheduler
