#include "options.hpp"

#include <client-io/io.hpp>
#include <client-io/registry.hpp>
#include <cxx11_compat.hpp>
#include <io/output.hpp>
#include <io/write_buffer.hpp>
#include <loglet/loglet.hpp>
#include <scheduler/file_descriptor.hpp>
#include <scheduler/scheduler.hpp>
#include <scheduler/socket.hpp>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

LOGLET_MODULE(ntrip_caster);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(ntrip_caster)

namespace {

// base64 decode, used for parsing "Authorization: Basic <...>" headers.
bool base64_decode(std::string const& input, std::string& output) {
    static int8_t sTable[256];
    static bool   sInitialized = false;
    if (!sInitialized) {
        std::memset(sTable, -1, sizeof(sTable));
        char const* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (int i = 0; i < 64; i++) {
            sTable[static_cast<uint8_t>(chars[i])] = static_cast<int8_t>(i);
        }
        sInitialized = true;
    }

    output.clear();
    uint32_t buffer = 0;
    int      bits   = 0;
    for (char c : input) {
        if (c == '=') break;
        auto value = sTable[static_cast<uint8_t>(c)];
        if (value < 0) continue;
        buffer = (buffer << 6) | static_cast<uint32_t>(value);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            output += static_cast<char>((buffer >> bits) & 0xff);
        }
    }

    return true;
}

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

std::string trim(std::string const& s) {
    size_t begin = 0;
    while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin])))
        begin++;
    size_t end = s.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1])))
        end--;
    return s.substr(begin, end - begin);
}

std::string sourcetable_line(MountpointOptions const& mp) {
    std::string line = "STR;";
    line += mp.mountpoint + ";";
    line += mp.identifier + ";";
    line += mp.format + ";";
    line += mp.format_details + ";";
    line += mp.carrier + ";";
    line += mp.nav_system + ";";
    line += mp.network + ";";
    line += mp.country + ";";
    char buf[64];
    snprintf(buf, sizeof(buf), "%.4f", mp.latitude);
    line += std::string(buf) + ";";
    snprintf(buf, sizeof(buf), "%.4f", mp.longitude);
    line += std::string(buf) + ";";
    line += (mp.nmea ? "1" : "0");
    line += ";";
    line += (mp.solution ? "1" : "0");
    line += ";";
    line += mp.generator + ";";
    line += mp.compr_encrp + ";";
    line += (mp.username_required ? "B" : "N");
    line += ";";
    line += "N;";  // fee
    line += mp.bitrate + ";";
    line += mp.misc;
    line += "\r\n";
    return line;
}

std::string build_sourcetable(CasterOptions const& opts) {
    std::string table;
    table += sourcetable_line(opts.mountpoint);
    table += "ENDSOURCETABLE\r\n";
    return table;
}

// RFC 1123 date, e.g. "Mon, 19 Oct 2026 13:05:03 UTC".
std::string http_date() {
    auto    now = std::time(nullptr);
    std::tm tm{};
    gmtime_r(&now, &tm);
    char buf[40];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S UTC", &tm);
    return std::string(buf);
}

std::string http_response(std::string const& status, std::string const& body) {
    std::string response = "HTTP/1.1 " + status + "\r\n";
    response += "Server: SUPL-3GPP-LPP-client/ntrip-caster\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    response += "Date: " + http_date() + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += body;
    return response;
}

// NTRIP sourcetable responses use a "SOURCETABLE <status>" status line
// instead of "HTTP/1.1 <status>", per the NTRIP 1.0 convention used by real
// casters (e.g. "SOURCETABLE 200 OK"). NTRIP 2.0 clients (those sending
// "Ntrip-Version: Ntrip/2.0") instead expect a standard HTTP status line
// with an "Ntrip-Version" header and a "gnss/sourcetable" content type —
// without this, HTTP-style clients that parse a numeric status code from
// the response fail to find one in "SOURCETABLE 200 OK" and treat the
// request as failed.
std::string sourcetable_response(std::string const& body, bool is_ntrip2) {
    std::string response;
    if (is_ntrip2) {
        response += "HTTP/1.1 200 OK\r\n";
        response += "Ntrip-Version: Ntrip/2.0\r\n";
        response += "Server: SUPL-3GPP-LPP-client/ntrip-caster\r\n";
        response += "Content-Type: gnss/sourcetable\r\n";
        response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        response += "Date: " + http_date() + "\r\n";
        response += "Connection: close\r\n";
    } else {
        response += "SOURCETABLE 200 OK\r\n";
        response += "Server: SUPL-3GPP-LPP-client/ntrip-caster\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        response += "Date: " + http_date() + "\r\n";
    }
    response += "\r\n";
    response += body;
    return response;
}

// A single connected client. Handles the initial NTRIP/HTTP request line and
// headers, then — once the request is validated — transitions into raw
// streaming mode where incoming bytes from --input are forwarded verbatim.
class Client {
public:
    Client(CasterOptions const& options, int fd) : mOptions(options), mFd(fd), mTask(fd) {
        mTask.set_event_name("ntrip-caster-client:" + std::to_string(fd));
        mTask.on_read = [this](scheduler::OwnedFileDescriptorTask&) {
            on_read();
        };
        mTask.on_write = [this](scheduler::OwnedFileDescriptorTask&) {
            on_write();
        };
        mTask.on_error = [this](scheduler::OwnedFileDescriptorTask&) {
            on_error();
        };
        (void)mTask.schedule();
    }

    ~Client() = default;

    Client(Client const&)            = delete;
    Client& operator=(Client const&) = delete;

    NODISCARD int  fd() const NOEXCEPT { return mFd; }
    NODISCARD bool is_streaming() const NOEXCEPT { return mStreaming; }
    NODISCARD bool failed() const NOEXCEPT { return mFailed; }

    std::function<void(int)> on_failed;

    void write(uint8_t const* data, size_t length) NOEXCEPT {
        if (!mStreaming || mFailed) return;
        if (mWriteBuffer.empty()) {
            auto result = ::write(mFd, data, length);
            if (result < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    result = 0;
                } else {
                    fail();
                    return;
                }
            }
            if (static_cast<size_t>(result) < length) {
                mWriteBuffer.enqueue(data + result, length - static_cast<size_t>(result));
                register_write_interest();
            }
        } else {
            mWriteBuffer.enqueue(data, length);
        }
    }

private:
    void fail() NOEXCEPT {
        if (mFailed) return;
        mFailed = true;
        // Stop watching the fd immediately — leaving a closed/hung-up
        // socket registered in epoll would make it permanently "ready",
        // starving other events until it is removed.
        if (mTask.is_scheduled()) mTask.cancel();
        auto fd = mFd;
        if (on_failed) {
            auto callback = on_failed;
            scheduler::defer([callback, fd](scheduler::Scheduler&) {
                callback(fd);
            });
        }
    }

    void register_write_interest() NOEXCEPT {
        if (mWriteRegistered) return;
        mTask.update_interests(scheduler::EventInterest::Read | scheduler::EventInterest::Write |
                               scheduler::EventInterest::Error | scheduler::EventInterest::Hangup);
        mWriteRegistered = true;
    }

    void on_read() NOEXCEPT {
        uint8_t buf[4096];
        auto    result = ::read(mFd, buf, sizeof(buf));
        if (result > 0) {
            if (mStreaming) {
                // Clients may send NMEA GGA updates after the stream starts;
                // this caster ignores them but keeps draining the socket so
                // it does not stall the connection.
                return;
            }
            mRequestBuffer.append(reinterpret_cast<char const*>(buf), static_cast<size_t>(result));
            if (mRequestBuffer.size() > 8192) {
                WARNF("fd=%d request too large, closing", mFd);
                fail();
                return;
            }
            try_handle_request();
        } else if (result == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
            fail();
        }
    }

    void on_write() NOEXCEPT {
        while (!mWriteBuffer.empty()) {
            auto peek   = mWriteBuffer.peek();
            auto result = ::write(mFd, peek.first, peek.second);
            if (result < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                fail();
                return;
            }
            mWriteBuffer.consume(static_cast<size_t>(result));
        }
        if (mWriteBuffer.empty() && mWriteRegistered) {
            mTask.update_interests(scheduler::EventInterest::Read |
                                   scheduler::EventInterest::Error |
                                   scheduler::EventInterest::Hangup);
            mWriteRegistered = false;
        }
    }

    void on_error() NOEXCEPT { fail(); }

    void try_handle_request() NOEXCEPT {
        auto header_end = mRequestBuffer.find("\r\n\r\n");
        auto is_ntrip1  = false;

        // NTRIP 1.0 clients send a bare request line terminated by a single
        // "\r\n" with no further headers, e.g. "GET /STREAM\r\n".
        auto line_end = mRequestBuffer.find("\r\n");
        if (line_end == std::string::npos) return;  // wait for more data

        if (header_end == std::string::npos) {
            // No full header block yet. If what we have so far looks like a
            // complete NTRIP 1.0 request (single line, no other headers
            // following), handle it directly instead of waiting forever for
            // a second CRLF that will never arrive.
            is_ntrip1 = true;
        }

        auto        request_line = mRequestBuffer.substr(0, line_end);
        std::string method, target;
        {
            auto sp1 = request_line.find(' ');
            if (sp1 == std::string::npos) {
                send_error_and_close("400 Bad Request", "Malformed request line");
                return;
            }
            method   = request_line.substr(0, sp1);
            auto sp2 = request_line.find(' ', sp1 + 1);
            target   = sp2 == std::string::npos ? request_line.substr(sp1 + 1) :
                                                  request_line.substr(sp1 + 1, sp2 - sp1 - 1);
        }

        if (method != "GET") {
            send_error_and_close("405 Method Not Allowed", "Only GET is supported");
            return;
        }

        std::string username, password;
        bool        has_auth  = false;
        bool        is_ntrip2 = false;
        if (!is_ntrip1) {
            // Parse headers up to header_end for Authorization and
            // Ntrip-Version.
            auto   headers_block = mRequestBuffer.substr(line_end + 2, header_end - (line_end + 2));
            size_t pos           = 0;
            while (pos < headers_block.size()) {
                auto next  = headers_block.find("\r\n", pos);
                auto line  = next == std::string::npos ? headers_block.substr(pos) :
                                                         headers_block.substr(pos, next - pos);
                auto colon = line.find(':');
                if (colon != std::string::npos) {
                    auto name  = to_lower(trim(line.substr(0, colon)));
                    auto value = trim(line.substr(colon + 1));
                    if (name == "authorization") {
                        static std::string const PREFIX = "Basic ";
                        if (value.compare(0, PREFIX.size(), PREFIX) == 0) {
                            std::string decoded;
                            base64_decode(value.substr(PREFIX.size()), decoded);
                            auto sep = decoded.find(':');
                            if (sep != std::string::npos) {
                                username = decoded.substr(0, sep);
                                password = decoded.substr(sep + 1);
                                has_auth = true;
                            }
                        }
                    } else if (name == "ntrip-version") {
                        if (value.find("2.0") != std::string::npos) is_ntrip2 = true;
                    }
                }
                if (next == std::string::npos) break;
                pos = next + 2;
            }
        }

        handle_target(target, has_auth, username, password, is_ntrip2);
    }

    void handle_target(std::string const& target, bool has_auth, std::string const& username,
                       std::string const& password, bool is_ntrip2) NOEXCEPT {
        auto path = target;
        if (!path.empty() && path[0] == '/') path = path.substr(1);

        if (path.empty()) {
            // Sourcetable request.
            auto body = build_sourcetable(mOptions);
            send_raw(sourcetable_response(body, is_ntrip2));
            fail();  // close after sending the sourcetable
            return;
        }

        if (path != mOptions.mountpoint.mountpoint) {
            send_error_and_close("404 Not Found", "Unknown mountpoint");
            return;
        }

        if (mOptions.mountpoint.username_required) {
            if (!has_auth || username != mOptions.username || password != mOptions.password) {
                send_error_and_close("401 Unauthorized", "Invalid credentials");
                return;
            }
        }

        // Accept — NTRIP 1.0/2.0 streaming clients expect a bare
        // "ICY 200 OK\r\n\r\n" with no further headers.
        send_raw("ICY 200 OK\r\n\r\n");
        mStreaming = true;
        INFOF("fd=%d streaming started", mFd);
    }

    void send_raw(std::string const& data) NOEXCEPT {
        auto result = ::write(mFd, data.data(), data.size());
        (void)result;
    }

    void send_error_and_close(std::string const& status, std::string const& body) NOEXCEPT {
        send_raw(http_response(status, body));
        fail();
    }

    CasterOptions const&               mOptions;
    int                                mFd;
    scheduler::OwnedFileDescriptorTask mTask;
    io::WriteBuffer                    mWriteBuffer;
    std::string                        mRequestBuffer;
    bool                               mWriteRegistered = false;
    bool                               mStreaming       = false;
    bool                               mFailed          = false;
};

}  // namespace

int main(int argc, char** argv) {
    io_registry::register_input_type(make_stdin_input_type());
    io_registry::register_input_type(make_file_input_type());
    io_registry::register_input_type(make_serial_input_type());
    io_registry::register_input_type(make_tcp_client_input_type());
    io_registry::register_input_type(make_tcp_server_input_type());
    io_registry::register_input_type(make_udp_server_input_type());
    io_registry::register_input_type(make_stream_ref_input_type());

    auto options = parse_configuration(argc, argv);
    loglet::initialize();
    loglet::set_level(loglet::Level::Info);
    loglet::set_always_flush(true);

    options.caster.mountpoint.username_required =
        !options.caster.username.empty() || !options.caster.password.empty();

    scheduler::Scheduler scheduler;
    scheduler::set_current(&scheduler);

    io::StreamRegistry registry;

    scheduler::TcpInetListenerTask listener{options.caster.address, options.caster.port};

    std::vector<std::unique_ptr<Client>> clients;

    auto remove_client = [&clients](int fd) {
        clients.erase(std::remove_if(clients.begin(), clients.end(),
                                     [fd](std::unique_ptr<Client> const& c) {
                                         return c->fd() == fd;
                                     }),
                      clients.end());
    };

    listener.on_accept = [&](scheduler::SocketListenerTask&, int fd, sockaddr_storage*, socklen_t) {
        DEBUGF("accepted new client connection, fd=%d", fd);
        auto client       = std::make_unique<Client>(options.caster, fd);
        client->on_failed = remove_client;
        clients.push_back(std::move(client));
    };
    listener.on_error = [&](scheduler::SocketListenerTask& task) {
        ERRORF("listener error on fd=%d", task.fd());
    };

    listener.schedule(scheduler);
    if (!listener.is_scheduled()) {
        ERRORF("failed to listen on %s:%u", options.caster.address.c_str(), options.caster.port);
        return 1;
    }

    INFOF("listening on %s:%u, mountpoint=/%s", options.caster.address.c_str(), options.caster.port,
          options.caster.mountpoint.mountpoint.c_str());

    std::vector<std::unique_ptr<io::Input>> inputs;
    auto                                    add_input = [&](std::unique_ptr<io::Input> inp) {
        if (!inp) return;
        inp->callback = [&clients](io::Input&, uint8_t* data, size_t len) {
            for (auto& client : clients) {
                client->write(data, len);
            }
        };
        (void)inp->schedule(scheduler);
        inputs.push_back(std::move(inp));
    };

    for (auto& entry : options.inputs.inputs) {
        add_input(create_input(entry, registry));
    }

    INFOF("caster running — %zu input(s)", inputs.size());

    scheduler.execute();
    return 0;
}
