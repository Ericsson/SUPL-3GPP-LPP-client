#include <doctest/doctest.h>
#include <io/stream/udp_client.hpp>
#include <io/stream/udp_server.hpp>
#include <scheduler/scheduler.hpp>

#include "test_helper.hpp"

#include <cstring>

TEST_CASE("UdpServerStream + UdpClientStream - loopback") {
    LogletTesting        loglet;
    scheduler::Scheduler scheduler;

    io::UdpServerConfig server_config;
    server_config.listen = "127.0.0.1";
    server_config.port   = 0;
    io::UdpServerStream server("server", server_config);
    REQUIRE(server.schedule(scheduler));

    auto port = server.actual_port();
    REQUIRE(port > 0);

    io::UdpClientConfig client_config;
    client_config.host = "127.0.0.1";
    client_config.port = port;
    io::UdpClientStream client("client", client_config);
    REQUIRE(client.schedule(scheduler));

    std::vector<uint8_t> server_recv;
    std::vector<uint8_t> client_recv;

    server.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        server_recv.insert(server_recv.end(), data, data + len);
    });
    client.on_read([&](io::Stream&, uint8_t* data, size_t len) {
        client_recv.insert(client_recv.end(), data, data + len);
    });

    uint8_t const msg1[] = "hello from client";
    client.write(msg1, sizeof(msg1));

    run_until_or_timeout(
        scheduler,
        [&] {
            return server_recv.size() >= sizeof(msg1);
        },
        std::chrono::milliseconds(2000));

    REQUIRE(server_recv.size() == sizeof(msg1));
    CHECK(memcmp(server_recv.data(), msg1, sizeof(msg1)) == 0);

    uint8_t const msg2[] = "hello from server";
    server.write(msg2, sizeof(msg2));

    run_until_or_timeout(
        scheduler,
        [&] {
            return client_recv.size() >= sizeof(msg2);
        },
        std::chrono::milliseconds(2000));

    REQUIRE(client_recv.size() == sizeof(msg2));
    CHECK(memcmp(client_recv.data(), msg2, sizeof(msg2)) == 0);
}
