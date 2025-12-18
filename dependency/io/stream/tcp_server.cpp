#include <io/stream/tcp_server.hpp>
#include <scheduler/socket.hpp>
#include <cxx11_compat.hpp>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE3(io, stream, tcp_server);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(io, stream, tcp_server)

namespace io {

TcpServerStream::TcpServerStream(std::string id, TcpServerConfig config) NOEXCEPT
    : Stream(std::move(id), config.read_config),
      mConfig(std::move(config)) {
    VSCOPE_FUNCTIONF("\"%s\", listen=\"%s\", port=%u, path=\"%s\"", mId.c_str(),
                     mConfig.listen.c_str(), mConfig.port, mConfig.path.c_str());
}

TcpServerStream::~TcpServerStream() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool TcpServerStream::schedule(scheduler::Scheduler& scheduler) {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    mScheduler = &scheduler;

    if (!mConfig.path.empty()) {
        mListenerTask.reset(new scheduler::TcpListenerTask(mConfig.path));
    } else {
        mListenerTask.reset(new scheduler::TcpListenerTask(mConfig.listen, mConfig.port));
    }

    mListenerTask->on_accept = [this](scheduler::TcpListenerTask&, int fd, sockaddr_storage*,
                                      socklen_t) {
        DEBUGF("accepted new client connection, fd=%d", fd);

        auto client  = std::make_unique<Client>();
        client->fd   = fd;
        client->task = std::make_unique<scheduler::SocketTask>(fd);
        client->task->set_event_name("tcp-server-client:" + mId);

        auto* client_ptr = client.get();

        client->task->on_read = [this, client_ptr](scheduler::SocketTask&) {
            auto result = ::read(client_ptr->fd, mReadBuf, sizeof(mReadBuf));
            VERBOSEF("::read(%d, %p, %zu) = %zd", client_ptr->fd, mReadBuf, sizeof(mReadBuf),
                     result);
            if (result > 0) {
                on_raw_read(mReadBuf, result);
            } else if (result == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
                if (result == 0) {
                    DEBUGF("client fd=%d disconnected", client_ptr->fd);
                } else {
                    WARNF("client fd=%d read error: " ERRNO_FMT, client_ptr->fd, ERRNO_ARGS(errno));
                }
                mScheduler->defer([this, client_ptr](scheduler::Scheduler&) {
                    remove_client(client_ptr);
                });
            }
        };

        client->task->on_write = [this, client_ptr](scheduler::SocketTask&) {
            while (!client_ptr->write_buffer.empty()) {
                auto [data, len] = client_ptr->write_buffer.peek();
                auto result      = ::write(client_ptr->fd, data, len);
                VERBOSEF("::write(%d, %p, %zu) = %zd", client_ptr->fd, data, len, result);
                if (result < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                    WARNF("client fd=%d write error: " ERRNO_FMT, client_ptr->fd,
                          ERRNO_ARGS(errno));
                    mScheduler->defer([this, client_ptr](scheduler::Scheduler&) {
                        remove_client(client_ptr);
                    });
                    return;
                }
                client_ptr->write_buffer.consume(result);
            }
            if (client_ptr->write_buffer.empty() && client_ptr->write_registered) {
                VERBOSEF("client fd=%d write buffer drained", client_ptr->fd);
                mScheduler->update_epoll_fd(client_ptr->fd, EPOLLIN, nullptr);
                client_ptr->write_registered = false;
            }
        };

        client->task->on_error = [this, client_ptr](scheduler::SocketTask&) {
            WARNF("client fd=%d socket error: " ERRNO_FMT, client_ptr->fd, ERRNO_ARGS(errno));
            mScheduler->defer([this, client_ptr](scheduler::Scheduler&) {
                remove_client(client_ptr);
            });
        };

        if (!client->task->schedule(*mScheduler)) {
            ERRORF("failed to schedule client task for fd=%d", fd);
            auto result = ::close(fd);
            VERBOSEF("::close(%d) = %d", fd, result);
            return;
        }

        mClients.push_back(std::move(client));
        VERBOSEF("total clients: %zu", mClients.size());
    };

    mListenerTask->on_error = [this](scheduler::TcpListenerTask&) {
        ERRORF("listener error: " ERRNO_FMT, ERRNO_ARGS(errno));
        set_error(errno, "listener error");
    };

    if (!mListenerTask->schedule(scheduler)) {
        ERRORF("failed to schedule TCP listener");
        set_error(errno, "failed to schedule TCP listener");
        return false;
    }

    if (!mConfig.path.empty()) {
        INFOF("listening on unix socket: %s", mConfig.path.c_str());
    } else {
        INFOF("listening on %s:%u", mConfig.listen.c_str(), mListenerTask->port());
    }

    mState = State::Connected;
    return schedule_read_timeout(scheduler);
}

bool TcpServerStream::cancel() {
    VSCOPE_FUNCTION();
    cancel_read_timeout();
    for (auto& client : mClients) {
        client->task->cancel();
        auto result = ::close(client->fd);
        VERBOSEF("::close(%d) = %d", client->fd, result);
    }
    mClients.clear();
    if (mListenerTask) {
        mListenerTask->cancel();
        mListenerTask.reset();
    }
    return true;
}

uint16_t TcpServerStream::actual_port() const NOEXCEPT {
    return mListenerTask ? mListenerTask->port() : 0;
}

void TcpServerStream::write(uint8_t const* data, size_t length) NOEXCEPT {
    TRACEF("%p, %zu to %zu clients", data, length, mClients.size());

    for (auto& client : mClients) {
        if (client->write_buffer.empty()) {
            auto result = ::write(client->fd, data, length);
            VERBOSEF("::write(%d, %p, %zu) = %zd", client->fd, data, length, result);
            if (result < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    result = 0;
                } else {
                    WARNF("client fd=%d write error: " ERRNO_FMT, client->fd, ERRNO_ARGS(errno));
                    continue;
                }
            }
            if (static_cast<size_t>(result) < length) {
                client->write_buffer.enqueue(data + result, length - result);
                if (!client->write_registered) {
                    mScheduler->update_epoll_fd(client->fd, EPOLLIN | EPOLLOUT, nullptr);
                    client->write_registered = true;
                }
            }
        } else {
            client->write_buffer.enqueue(data, length);
        }
    }
}

void TcpServerStream::remove_client(Client* client) NOEXCEPT {
    DEBUGF("removing client fd=%d", client->fd);
    client->task->cancel();
    auto result = ::close(client->fd);
    VERBOSEF("::close(%d) = %d", client->fd, result);
    mClients.erase(std::remove_if(mClients.begin(), mClients.end(),
                                  [client](std::unique_ptr<Client>& c) {
                                      return c.get() == client;
                                  }),
                   mClients.end());
    VERBOSEF("remaining clients: %zu", mClients.size());
}

}  // namespace io
