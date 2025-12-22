#include <doctest/doctest.h>
#include <io/stream/tcp_client.hpp>
#include <io/stream/tcp_server.hpp>
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>
#include <scheduler/socket.hpp>

#include "test_helper.hpp"

#include <cstring>

TEST_CASE("TcpServerStream + TcpClientStream - loopback") {
    scheduler::ScopedScheduler scheduler;

    auto                listener = std::make_unique<scheduler::TcpInetListenerTask>("127.0.0.1", 0);
    io::TcpServerStream server("server", std::move(listener));
    REQUIRE(server.schedule(scheduler));

    auto port = server.port();
    REQUIRE(port > 0);

    io::TcpClientConfig client_config;
    client_config.host = "127.0.0.1";
    client_config.port = port;
    io::TcpClientStream client("client", client_config);
    REQUIRE(client.schedule(scheduler));

    run_until_or_timeout(
        scheduler,
        [&] {
            return client.state() == io::Stream::State::Connected;
        },
        std::chrono::milliseconds(2000));
    REQUIRE(client.state() == io::Stream::State::Connected);

    std::vector<uint8_t> server_recv;
    std::vector<uint8_t> client_recv;

    server.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        server_recv.insert(server_recv.end(), data, data + len);
    });
    client.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        client_recv.insert(client_recv.end(), data, data + len);
    });

    uint8_t const msg1[] = "hello from client";
    uint8_t const msg2[] = "hello from server";

    client.write(msg1, sizeof(msg1));
    server.write(msg2, sizeof(msg2));

    run_until_or_timeout(
        scheduler,
        [&] {
            return server_recv.size() >= sizeof(msg1) && client_recv.size() >= sizeof(msg2);
        },
        std::chrono::milliseconds(2000));

    REQUIRE(server_recv.size() == sizeof(msg1));
    REQUIRE(client_recv.size() == sizeof(msg2));
    CHECK(memcmp(server_recv.data(), msg1, sizeof(msg1)) == 0);
    CHECK(memcmp(client_recv.data(), msg2, sizeof(msg2)) == 0);
}

TEST_CASE("TcpServerStream - multiple clients") {
    scheduler::ScopedScheduler scheduler;

    auto                listener = std::make_unique<scheduler::TcpInetListenerTask>("127.0.0.1", 0);
    io::TcpServerStream server("server", std::move(listener));
    REQUIRE(server.schedule(scheduler));

    auto port = server.port();
    REQUIRE(port > 0);

    io::TcpClientConfig client_config;
    client_config.host = "127.0.0.1";
    client_config.port = port;

    io::TcpClientStream client1("client1", client_config);
    io::TcpClientStream client2("client2", client_config);
    io::TcpClientStream client3("client3", client_config);

    REQUIRE(client1.schedule(scheduler));
    REQUIRE(client2.schedule(scheduler));
    REQUIRE(client3.schedule(scheduler));

    run_until_or_timeout(
        scheduler,
        [&] {
            return client1.state() == io::Stream::State::Connected &&
                   client2.state() == io::Stream::State::Connected &&
                   client3.state() == io::Stream::State::Connected;
        },
        std::chrono::milliseconds(2000));

    REQUIRE(client1.state() == io::Stream::State::Connected);
    REQUIRE(client2.state() == io::Stream::State::Connected);
    REQUIRE(client3.state() == io::Stream::State::Connected);

    std::vector<uint8_t> server_recv;
    std::vector<uint8_t> client1_recv;
    std::vector<uint8_t> client2_recv;
    std::vector<uint8_t> client3_recv;

    server.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        server_recv.insert(server_recv.end(), data, data + len);
    });
    client1.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        client1_recv.insert(client1_recv.end(), data, data + len);
    });
    client2.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        client2_recv.insert(client2_recv.end(), data, data + len);
    });
    client3.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        client3_recv.insert(client3_recv.end(), data, data + len);
    });

    uint8_t const msg1[]      = "from client1";
    uint8_t const msg2[]      = "from client2";
    uint8_t const msg3[]      = "from client3";
    uint8_t const broadcast[] = "broadcast to all";

    client1.write(msg1, sizeof(msg1));
    client2.write(msg2, sizeof(msg2));
    client3.write(msg3, sizeof(msg3));

    run_until_or_timeout(
        scheduler,
        [&] {
            return server_recv.size() >= sizeof(msg1) + sizeof(msg2) + sizeof(msg3);
        },
        std::chrono::milliseconds(2000));

    REQUIRE(server_recv.size() == sizeof(msg1) + sizeof(msg2) + sizeof(msg3));

    server.write(broadcast, sizeof(broadcast));

    run_until_or_timeout(
        scheduler,
        [&] {
            return client1_recv.size() >= sizeof(broadcast) &&
                   client2_recv.size() >= sizeof(broadcast) &&
                   client3_recv.size() >= sizeof(broadcast);
        },
        std::chrono::milliseconds(2000));

    CHECK(client1_recv.size() == sizeof(broadcast));
    CHECK(client2_recv.size() == sizeof(broadcast));
    CHECK(client3_recv.size() == sizeof(broadcast));
    CHECK(memcmp(client1_recv.data(), broadcast, sizeof(broadcast)) == 0);
    CHECK(memcmp(client2_recv.data(), broadcast, sizeof(broadcast)) == 0);
    CHECK(memcmp(client3_recv.data(), broadcast, sizeof(broadcast)) == 0);
}

TEST_CASE("TcpServerStream - connection churn") {
    scheduler::ScopedScheduler scheduler;

    auto                listener = std::make_unique<scheduler::TcpInetListenerTask>("127.0.0.1", 0);
    io::TcpServerStream server("server", std::move(listener));
    REQUIRE(server.schedule(scheduler));

    auto port = server.port();
    REQUIRE(port > 0);

    io::TcpClientConfig client_config;
    client_config.host = "127.0.0.1";
    client_config.port = port;

    size_t total_received = 0;
    server.on_read([&](io::Stream&, uint8_t*, size_t len) {
        total_received += len;
    });

    for (int round = 0; round < 10; ++round) {
        std::vector<std::unique_ptr<io::TcpClientStream>> clients;

        for (int i = 0; i < 5; ++i) {
            auto client =
                std::make_unique<io::TcpClientStream>("client-" + std::to_string(i), client_config);
            REQUIRE(client->schedule(scheduler));
            clients.push_back(std::move(client));
        }

        run_until_or_timeout(
            scheduler,
            [&] {
                return std::all_of(clients.begin(), clients.end(), [](auto& c) {
                    return c->state() == io::Stream::State::Connected;
                });
            },
            std::chrono::milliseconds(2000));

        for (auto& client : clients) {
            REQUIRE(client->state() == io::Stream::State::Connected);
            uint8_t msg[] = "test";
            client->write(msg, sizeof(msg));
        }

        run_until_or_timeout(
            scheduler,
            [&] {
                return total_received >= (round + 1) * 5 * 5;
            },
            std::chrono::milliseconds(2000));

        clients.clear();

        run_until_or_timeout(
            scheduler,
            [] {
                return false;
            },
            std::chrono::milliseconds(50));
    }

    CHECK(total_received == 10 * 5 * 5);
}

TEST_CASE("TcpServerStream - periodic client connections") {
    scheduler::ScopedScheduler scheduler;

    auto                listener = std::make_unique<scheduler::TcpInetListenerTask>("127.0.0.1", 0);
    io::TcpServerStream server("server", std::move(listener));
    REQUIRE(server.schedule(scheduler));

    io::TcpClientConfig client_config;
    client_config.host = "127.0.0.1";
    client_config.port = server.port();

    int  connections_made = 0;
    int  disconnections   = 0;
    auto clients          = std::vector<std::unique_ptr<io::TcpClientStream>>{};

    scheduler::PeriodicTask connect_timer(std::chrono::milliseconds(5));
    connect_timer.callback = [&] {
        auto client = std::make_unique<io::TcpClientStream>(
            "client-" + std::to_string(connections_made), client_config);
        (void)client->schedule(scheduler);
        clients.push_back(std::move(client));
        ++connections_made;
        if (connections_made >= 100) connect_timer.cancel();
    };
    connect_timer.schedule();

    scheduler::PeriodicTask disconnect_timer(std::chrono::milliseconds(7));
    disconnect_timer.callback = [&] {
        if (!clients.empty()) {
            clients.erase(clients.begin());
            ++disconnections;
        }
        if (disconnections >= 100) {
            disconnect_timer.cancel();
            scheduler.interrupt();
        }
    };
    disconnect_timer.schedule();

    auto result = scheduler.execute_timeout(std::chrono::seconds(10));
    REQUIRE(result == scheduler::ExecuteResult::Interrupted);

    CHECK(connections_made == 100);
    CHECK(disconnections == 100);
    CHECK(clients.empty());
}
