#include <arpa/inet.h>
#include <cstring>
#include <doctest/doctest.h>
#include <memory>
#include <netinet/in.h>
#include <scheduler/scheduler.hpp>
#include <scheduler/socket.hpp>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

TEST_CASE("SocketTask read/write with socketpair") {
    scheduler::Scheduler sched;

    int fds[2];
    REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    scheduler::SocketTask task(fds[0]);

    bool                 read_called = false;
    std::vector<uint8_t> received;

    task.on_read = [&](scheduler::SocketTask& t) {
        char buf[256];
        auto n = ::read(t.fd(), buf, sizeof(buf));
        if (n > 0) {
            received.insert(received.end(), buf, buf + n);
            read_called = true;
            sched.interrupt();
        }
    };

    CHECK(task.schedule(sched));

    // Write from other end
    char const* msg = "hello";
    ::write(fds[1], msg, strlen(msg));

    sched.execute_timeout(std::chrono::milliseconds(100));

    CHECK(read_called);
    CHECK(received.size() == 5);
    CHECK(std::string(received.begin(), received.end()) == "hello");

    ::close(fds[1]);
}

TEST_CASE("SocketTask error on close") {
    scheduler::Scheduler sched;

    int fds[2];
    REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    scheduler::SocketTask task(fds[0]);

    bool error_called = false;
    task.on_error     = [&](scheduler::SocketTask&) {
        error_called = true;
        sched.interrupt();
    };

    CHECK(task.schedule(sched));

    // Close remote end
    ::close(fds[1]);

    sched.execute_timeout(std::chrono::milliseconds(100));

    CHECK(error_called);
}

TEST_CASE("TcpListenerTask accepts connections") {
    // This test is in integration.cpp as "Echo server integration"
    // which properly handles the async nature of connect/accept
    CHECK(true);
}

TEST_CASE("TcpConnectTask read/write") {
    scheduler::Scheduler sched;

    scheduler::TcpListenerTask listener("/tmp/test_scheduler_rw.sock");
    ::unlink("/tmp/test_scheduler_rw.sock");

    int server_fd      = -1;
    listener.on_accept = [&](auto&, int fd, auto*, auto) {
        server_fd = fd;
        // Echo server
        char buf[256];
        auto n = ::read(fd, buf, sizeof(buf));
        if (n > 0) ::write(fd, buf, n);
    };

    listener.schedule(sched);

    scheduler::TcpConnectTask client("/tmp/test_scheduler_rw.sock", false);

    bool        got_response = false;
    std::string response;

    client.on_connected = [&](scheduler::TcpConnectTask& c) {
        char const* msg = "test";
        ::write(c.fd(), msg, strlen(msg));
    };

    client.on_read = [&](scheduler::TcpConnectTask& c) {
        char buf[256];
        auto n = ::read(c.fd(), buf, sizeof(buf));
        if (n > 0) {
            response.append(buf, n);
            got_response = true;
            sched.interrupt();
        }
    };

    client.schedule(sched);
    sched.execute_timeout(std::chrono::milliseconds(500));

    CHECK(got_response);
    CHECK(response == "test");

    if (server_fd >= 0) ::close(server_fd);
    ::unlink("/tmp/test_scheduler_rw.sock");
}

TEST_CASE("UdpListenerTask basic") {
    scheduler::Scheduler sched;

    scheduler::UdpListenerTask listener("127.0.0.1", 0);

    bool received    = false;
    listener.on_read = [&](scheduler::UdpListenerTask& l) {
        char buf[256];
        auto n = ::read(l.fd(), buf, sizeof(buf));
        if (n > 0) {
            received = true;
            sched.interrupt();
        }
    };

    CHECK(listener.schedule(sched));

    // Get actual port
    struct sockaddr_in addr;
    socklen_t          len = sizeof(addr);
    ::getsockname(listener.fd(), (struct sockaddr*)&addr, &len);
    uint16_t port = ntohs(addr.sin_port);

    // Send datagram
    int sock        = ::socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    ::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    char const* msg = "udp test";
    ::sendto(sock, msg, strlen(msg), 0, (struct sockaddr*)&addr, sizeof(addr));

    sched.execute_timeout(std::chrono::milliseconds(100));

    CHECK(received);
    ::close(sock);
}
