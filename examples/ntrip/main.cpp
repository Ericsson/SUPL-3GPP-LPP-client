#include <arpa/inet.h>
#include <chrono>
#include <client-io/io.hpp>
#include <client-io/registry.hpp>
#include <cstring>
#include <deque>
#include <exception>
#include <loglet/loglet.hpp>
#include <netdb.h>
#include <poll.h>
#include <scheduler/scheduler.hpp>
#include <stdexcept>
#include <unistd.h>
#include <vector>
#include "options.hpp"

LOGLET_MODULE(ntrip);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(ntrip)

// resolve sockaddr from hostname
static sockaddr_in resolve(std::string const& hostname, uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);

    auto host = gethostbyname(hostname.c_str());
    if (!host) throw std::runtime_error("Failed to resolve hostname");

    memcpy(&addr.sin_addr, host->h_addr_list[0], static_cast<size_t>(host->h_length));
    return addr;
}

static std::string base64_encode(uint8_t* data, size_t size) {
    static char const* sTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded;
    encoded.reserve((size + 2) / 3 * 4);

    for (size_t i = 0; i < size; i += 3) {
        uint32_t temp = 0;
        temp |= static_cast<uint32_t>(data[i + 0] << 16);
        if (i + 1 < size) temp |= static_cast<uint32_t>(data[i + 1] << 8);
        if (i + 2 < size) temp |= static_cast<uint32_t>(data[i + 2] << 0);

        encoded += sTable[(temp >> 18) & 0x3f];
        encoded += sTable[(temp >> 12) & 0x3f];
        if (i + 1 < size) {
            encoded += sTable[(temp >> 6) & 0x3f];
        } else {
            encoded += '=';
        }
        if (i + 2 < size) {
            encoded += sTable[(temp >> 0) & 0x3f];
        } else {
            encoded += '=';
        }
    }

    return encoded;
}

// username:password -> base64
static std::string authorization_basic(std::string const& username, std::string const& password) {
    auto data = username + ":" + password;
    auto size = data.size();
    auto temp = reinterpret_cast<uint8_t*>(malloc(size));
    memcpy(temp, data.c_str(), size);
    auto encoded = base64_encode(temp, size);
    free(temp);
    return encoded;
}

// hexdump with ascii
static void hexdump(void const* data, size_t size) {
    auto bytes = reinterpret_cast<uint8_t const*>(data);
    for (size_t i = 0; i < size; i += 16) {
        printf("%08zx: ", i);

        // hex
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                printf("%02x ", bytes[i + j]);
            } else {
                printf("   ");
            }
        }

        // ascii
        printf(" ");
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                auto c = bytes[i + j];
                if (c >= 0x20 && c <= 0x7e) {
                    printf("%c", c);
                } else {
                    printf(".");
                }
            } else {
                printf(" ");
            }
        }

        printf("\n");
    }
}

class Ntrip {
public:
    Ntrip(sockaddr_in addr) {
        mSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (mSocket < 0) throw std::runtime_error("Failed to create socket");

        auto socket_addr = reinterpret_cast<sockaddr*>(&addr);
        auto socket_len  = static_cast<socklen_t>(sizeof(addr));
        if (connect(mSocket, socket_addr, socket_len) < 0) {
            throw std::runtime_error("Failed to connect to host");
        }
    }

    ~Ntrip() { close(mSocket); }

    void authorize(std::string username, std::string password) {
        mUsername = username;
        mPassword = password;
    }

    void request(std::string const& mountpoint) {
        std::string request = "GET /" + mountpoint + " HTTP/1.0\r\n";
        request += "User-Agent: NTRIP 2.0/SUPL-3GPP-LPP-client\r\n";
        request += "Accept: */*\r\n";
        request += "Connection: close\r\n";
        if (!mUsername.empty() && !mPassword.empty()) {
            request += "Authorization: Basic " + authorization_basic(mUsername, mPassword) + "\r\n";
        }
        request += "\r\n";

        if (send(mSocket, request.c_str(), request.size(), 0) < 0) {
            throw std::runtime_error("Failed to send request");
        }
    }

    void nmea_update(std::string const& nmea) {
        std::string request = nmea + "\r\n";
        if (send(mSocket, request.c_str(), request.size(), 0) < 0) {
            throw std::runtime_error("Failed to send request");
        }
    }

    ssize_t read(void* buffer, size_t size) { return ::read(mSocket, buffer, size); }

    NODISCARD int fd() const NOEXCEPT { return mSocket; }

private:
    int         mSocket;
    std::string mUsername;
    std::string mPassword;
};

// Buffers incoming chunks and releases them to the outputs once they are
// older than the configured delay, preserving arrival order.
class DelayBuffer {
public:
    EXPLICIT DelayBuffer(std::chrono::milliseconds delay) NOEXCEPT : mDelay(delay) {}

    void push(uint8_t const* data, size_t length) {
        mChunks.push_back(
            Chunk{std::vector<uint8_t>(data, data + length), std::chrono::steady_clock::now()});
    }

    // Invokes `emit` for every chunk that has aged past the configured delay.
    template <typename Emit>
    void flush_due(Emit&& emit) {
        auto now = std::chrono::steady_clock::now();
        while (!mChunks.empty() && now - mChunks.front().arrival >= mDelay) {
            emit(mChunks.front().data.data(), mChunks.front().data.size());
            mChunks.pop_front();
        }
    }

    // Invokes `emit` for every remaining chunk, regardless of age (used to
    // drain the buffer once the upstream connection ends).
    template <typename Emit>
    void flush_all(Emit&& emit) {
        while (!mChunks.empty()) {
            emit(mChunks.front().data.data(), mChunks.front().data.size());
            mChunks.pop_front();
        }
    }

    NODISCARD bool empty() const NOEXCEPT { return mChunks.empty(); }

    // Time until the oldest buffered chunk becomes due, or zero if none are
    // buffered or one is already due.
    NODISCARD std::chrono::milliseconds time_until_next() const NOEXCEPT {
        if (mChunks.empty()) return std::chrono::milliseconds(0);
        auto age       = std::chrono::steady_clock::now() - mChunks.front().arrival;
        auto remaining = mDelay - age;
        if (remaining < std::chrono::milliseconds(0)) return std::chrono::milliseconds(0);
        return std::chrono::duration_cast<std::chrono::milliseconds>(remaining);
    }

private:
    struct Chunk {
        std::vector<uint8_t>                  data;
        std::chrono::steady_clock::time_point arrival;
    };

    std::chrono::milliseconds mDelay;
    std::deque<Chunk>         mChunks;
};

int main(int argc, char** argv) {
    // Register output types before parse_configuration calls output::setup()
    io_registry::register_output_type(make_stdout_output_type());
    io_registry::register_output_type(make_file_output_type());
    io_registry::register_output_type(make_tcp_server_output_type());
    io_registry::register_output_type(make_serial_output_type());
    io_registry::register_output_type(make_tcp_client_output_type());
    io_registry::register_output_type(make_udp_client_output_type());
    io_registry::register_output_type(make_stream_ref_output_type());

    auto options = parse_configuration(argc, argv);
    loglet::set_level(loglet::Level::Info);
    loglet::set_always_flush(true);
    auto& host = options.host;

    // Build outputs
    io::StreamRegistry                       registry;
    std::vector<std::unique_ptr<io::Output>> outputs;
    scheduler::Scheduler                     scheduler;
    scheduler::set_current(&scheduler);

    auto add = [&](std::unique_ptr<io::Output> o) {
        if (!o) return;
        (void)o->schedule(scheduler);
        outputs.push_back(std::move(o));
    };
    for (auto& entry : options.outputs.outputs)
        add(create_output(entry, registry));

    // resolve hostname
    auto addr = resolve(host.hostname, host.port);

    // connect to host
    INFOF("connecting to %s:%d", host.hostname.c_str(), host.port);
    Ntrip ntrip(addr);
    ntrip.authorize(host.username, host.password);

    if (host.mountpoint) {
        INFOF("requesting mountpoint: %s", host.mountpoint->c_str());
        ntrip.request(*host.mountpoint);
    } else {
        ntrip.request("");
    }

    if (!host.nmea.empty()) {
        ntrip.nmea_update(host.nmea);
    }

    INFOF("streaming — %zu output(s)", outputs.size());
    if (options.delay.count() > 0) {
        INFOF("delaying forwarded data by %lld ms", static_cast<long long>(options.delay.count()));
    }

    DelayBuffer delay_buffer(options.delay);
    auto        emit = [&](uint8_t const* data, size_t length) {
        for (auto& out : outputs) {
            out->write(data, length);
        }
    };

    size_t total_bytes = 0;
    char   temp[4096];
    for (;;) {
        // Wait for incoming data, but wake up early if a delayed chunk is due
        // to be flushed even when no new data arrives.
        struct pollfd pfd{};
        pfd.fd     = ntrip.fd();
        pfd.events = POLLIN;

        auto wait_ms     = delay_buffer.time_until_next();
        auto timeout_ms  = options.delay.count() > 0 ? static_cast<int>(wait_ms.count()) : -1;
        auto poll_result = ::poll(&pfd, 1, timeout_ms);
        if (poll_result < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error("Failed to poll NTRIP socket");
        }

        if (poll_result > 0 && (pfd.revents & (POLLIN | POLLERR | POLLHUP))) {
            auto bytes = ntrip.read(temp, sizeof(temp));
            if (bytes < 0) {
                throw std::runtime_error("Failed to read from NTRIP");
            } else if (bytes == 0) {
                break;
            }

            auto length = static_cast<size_t>(bytes);
            total_bytes += length;
            if (host.hexdump) {
                hexdump(temp, length);
            }

            if (options.delay.count() > 0) {
                delay_buffer.push(reinterpret_cast<uint8_t*>(temp), length);
            } else {
                emit(reinterpret_cast<uint8_t*>(temp), length);
            }
        }

        delay_buffer.flush_due(emit);
        (void)scheduler.execute_once();
    }

    // Drain any remaining delayed data before exiting.
    delay_buffer.flush_all(emit);

    INFOF("done — %zu bytes total", total_bytes);
}
