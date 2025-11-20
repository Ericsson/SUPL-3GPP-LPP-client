#include <cstring>
#include <cxx11_compat.hpp>
#include <doctest/doctest.h>
#include <scheduler/scheduler.hpp>
#include <scheduler/socket.hpp>
#include <scheduler/timeout.hpp>
#include <sys/socket.h>
#include <unistd.h>

TEST_CASE("Echo server integration") {
    scheduler::Scheduler sched;

    // Server
    scheduler::TcpListenerTask server("/tmp/echo_server.sock");
    ::unlink("/tmp/echo_server.sock");

    std::unique_ptr<scheduler::SocketTask> server_conn;

    server.on_accept = [&](scheduler::TcpListenerTask&, int fd, struct sockaddr_storage*,
                           socklen_t) {
        server_conn = std::make_unique<scheduler::SocketTask>(fd);

        server_conn->on_read = [](scheduler::SocketTask& task) {
            char buf[256];
            auto n = ::read(task.fd(), buf, sizeof(buf));
            if (n > 0) {
                ::write(task.fd(), buf, n);  // Echo back
            }
        };

        server_conn->schedule(sched);
    };

    server.schedule(sched);

    // Client
    scheduler::TcpConnectTask client("/tmp/echo_server.sock", false);

    std::string received;
    bool        done = false;

    client.on_connected = [](scheduler::TcpConnectTask& c) {
        char const* msg = "Hello, Echo!";
        ::write(c.fd(), msg, strlen(msg));
    };

    client.on_read = [&](scheduler::TcpConnectTask& c) {
        char buf[256];
        auto n = ::read(c.fd(), buf, sizeof(buf));
        if (n > 0) {
            received.append(buf, n);
            done = true;
            sched.interrupt();
        }
    };

    client.schedule(sched);
    sched.execute_timeout(std::chrono::seconds(1));

    CHECK(done);
    CHECK(received == "Hello, Echo!");

    ::unlink("/tmp/echo_server.sock");
}

TEST_CASE("Multiple clients to one server") {
    scheduler::Scheduler sched;

    scheduler::TcpListenerTask server("/tmp/multi_server.sock");
    ::unlink("/tmp/multi_server.sock");

    std::vector<std::unique_ptr<scheduler::SocketTask>> connections;

    server.on_accept = [&](scheduler::TcpListenerTask&, int fd, struct sockaddr_storage*,
                           socklen_t) {
        auto conn = std::make_unique<scheduler::SocketTask>(fd);

        conn->on_read = [](scheduler::SocketTask& task) {
            char buf[256];
            auto n = ::read(task.fd(), buf, sizeof(buf));
            if (n > 0) {
                ::write(task.fd(), "ACK", 3);
            }
        };

        conn->schedule(sched);
        connections.push_back(std::move(conn));
    };

    server.schedule(sched);

    // Create 5 clients
    std::vector<std::unique_ptr<scheduler::TcpConnectTask>> clients;
    std::vector<bool>                                       responses(5, false);

    for (int i = 0; i < 5; i++) {
        auto client = std::make_unique<scheduler::TcpConnectTask>("/tmp/multi_server.sock", false);

        client->on_connected = [](scheduler::TcpConnectTask& c) {
            ::write(c.fd(), "PING", 4);
        };

        client->on_read = [&responses, i, &sched](scheduler::TcpConnectTask& c) {
            char buf[256];
            auto n = ::read(c.fd(), buf, sizeof(buf));
            if (n == 3 && std::string(buf, 3) == "ACK") {
                responses[i] = true;

                bool all_done = true;
                for (auto r : responses) {
                    if (!r) {
                        all_done = false;
                        break;
                    }
                }
                if (all_done) sched.interrupt();
            }
        };

        client->schedule(sched);
        clients.push_back(std::move(client));
    }

    sched.execute_timeout(std::chrono::seconds(2));

    for (int i = 0; i < 5; i++) {
        CHECK(responses[i]);
    }

    ::unlink("/tmp/multi_server.sock");
}

TEST_CASE("Timeout during socket operation") {
    scheduler::Scheduler sched;

    int fds[2];
    REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    scheduler::SocketTask task(fds[0]);

    bool read_called   = false;
    bool timeout_fired = false;

    task.on_read = [&](scheduler::SocketTask&) {
        read_called = true;
    };

    task.schedule(sched);

    scheduler::TimeoutTask timeout(std::chrono::milliseconds(50));
    timeout.callback = [&]() {
        timeout_fired = true;
        sched.interrupt();
    };
    timeout.schedule(sched);

    // Don't write anything - socket won't be readable
    sched.execute();

    CHECK_FALSE(read_called);
    CHECK(timeout_fired);

    ::close(fds[0]);
    ::close(fds[1]);
}

TEST_CASE("Reconnection after disconnect") {
    scheduler::Scheduler sched;

    scheduler::TcpListenerTask server("/tmp/reconnect_server.sock");
    ::unlink("/tmp/reconnect_server.sock");

    int                                                 accept_count = 0;
    std::vector<std::unique_ptr<scheduler::SocketTask>> connections;

    server.on_accept = [&](scheduler::TcpListenerTask&, int fd, struct sockaddr_storage*,
                           socklen_t) {
        accept_count++;
        auto conn = std::make_unique<scheduler::SocketTask>(fd);

        if (accept_count == 1) {
            // Close first connection immediately
            ::close(fd);
        } else {
            conn->on_read = [&sched](scheduler::SocketTask& task) {
                char buf[1];
                ::read(task.fd(), buf, 1);
                sched.interrupt();
            };
            conn->schedule(sched);
            connections.push_back(std::move(conn));
        }
    };

    server.schedule(sched);

    scheduler::TcpConnectTask client("/tmp/reconnect_server.sock", true);

    int connect_count   = 0;
    client.on_connected = [&](scheduler::TcpConnectTask& c) {
        connect_count++;
        if (connect_count == 2) {
            ::write(c.fd(), "x", 1);
        }
    };

    client.schedule(sched);
    sched.execute_timeout(std::chrono::seconds(15));

    CHECK(accept_count >= 2);
    CHECK(connect_count >= 2);

    ::unlink("/tmp/reconnect_server.sock");
}
