#include "tcp.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <scheduler/scheduler.hpp>
#include <scheduler/socket.hpp>

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "io"

namespace io {
TcpServerInput::TcpServerInput(std::string listen, uint16_t port) NOEXCEPT
    : mListen(std::move(listen)),
      mPort(port) {
    VSCOPE_FUNCTIONF("\"%s\", %u", mListen.c_str(), mPort);
}

TcpServerInput::~TcpServerInput() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool TcpServerInput::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    mListenerTask.reset(new scheduler::TcpListenerTask(mListen, mPort));
    mListenerTask->on_accept = [this, &scheduler](scheduler::TcpListenerTask&, int data_fd,
                                                  struct sockaddr_storage*, socklen_t) {
        auto it = mClientTasks.begin();
        while (it != mClientTasks.end()) {
            if (!(*it)->is_scheduled()) {
                VERBOSEF("remove client task: %p", it->get());
                it = mClientTasks.erase(it);
            } else {
                ++it;
            }
        }

        auto client = std::unique_ptr<scheduler::SocketTask>(new scheduler::SocketTask(data_fd));
        client->on_read = [this](scheduler::SocketTask& task) {
            auto result = ::read(task.fd(), mBuffer, sizeof(mBuffer));
            VERBOSEF("::read(%d, %p, %zu) = %d", task.fd(), mBuffer, sizeof(mBuffer), result);
            if (result < 0) {
                ERRORF("failed to read from socket: " ERRNO_FMT, ERRNO_ARGS(errno));
                task.cancel();
                return;
            }

            if (callback) {
                callback(*this, mBuffer, static_cast<size_t>(result));
            }
        };
        client->on_error = [](scheduler::SocketTask& task) {
            // NOTE: I am not sure what to do here.
            task.cancel();
        };

        if (!client->schedule(scheduler)) {
            auto result = ::close(data_fd);
            VERBOSEF("::close(%d) = %d", data_fd, result);
        } else {
            VERBOSEF("add client task: %p", client.get());
            mClientTasks.push_back(std::move(client));
        }
    };
    mListenerTask->on_error = [this](scheduler::TcpListenerTask&) {
        // NOTE: I am not sure what to do here.
        cancel();
    };

    if (!mListenerTask->schedule(scheduler)) {
        mListenerTask.reset();
        return false;
    }

    return true;
}

bool TcpServerInput::do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    if (mListenerTask) {
        mListenerTask->cancel();
        mListenerTask.reset();
    }

    return true;
}

//
//
//

TcpClientInput::TcpClientInput(std::string host, uint16_t port) NOEXCEPT
    : mHost(std::move(host)),
      mPort(port) {
    VSCOPE_FUNCTIONF("\"%s\", %u", mHost.c_str(), mPort);
}

TcpClientInput::~TcpClientInput() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool TcpClientInput::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    mConnectTask.reset(new scheduler::TcpConnectTask(mHost, mPort));
    mConnectTask->on_read = [this](scheduler::TcpConnectTask& task) {
        auto result = ::read(task.fd(), mBuffer, sizeof(mBuffer));
        VERBOSEF("::read(%d, %p, %zu) = %d", task.fd(), mBuffer, sizeof(mBuffer), result);
        if (result < 0) {
            ERRORF("failed to read from socket: " ERRNO_FMT, ERRNO_ARGS(errno));
            mConnectTask->cancel();
            return;
        }

        if (callback) {
            callback(*this, mBuffer, result);
        }
    };

    if (!mConnectTask->schedule(scheduler)) {
        mConnectTask.reset();
        return false;
    }

    return true;
}

bool TcpClientInput::do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    if (mConnectTask) {
        mConnectTask->cancel();
        mConnectTask.reset();
    }

    return true;
}
}  // namespace io
