#include <arpa/inet.h>
#include <cstring>
#include <exception>
#include <netdb.h>
#include <stdexcept>
#include <unistd.h>
#include "options.hpp"

// resolve sockaddr from hostname
static sockaddr_in resolve(const std::string& hostname, uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);

    auto host = gethostbyname(hostname.c_str());
    if (!host) throw std::runtime_error("Failed to resolve hostname");

    memcpy(&addr.sin_addr, host->h_addr_list[0], host->h_length);
    return addr;
}

static std::string base64_encode(uint8_t* data, size_t size) {
    static const char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded;
    encoded.reserve((size + 2) / 3 * 4);

    for (size_t i = 0; i < size; i += 3) {
        uint32_t temp = 0;
        temp |= data[i + 0] << 16;
        if (i + 1 < size) temp |= data[i + 1] << 8;
        if (i + 2 < size) temp |= data[i + 2] << 0;

        encoded += table[(temp >> 18) & 0x3f];
        encoded += table[(temp >> 12) & 0x3f];
        if (i + 1 < size) {
            encoded += table[(temp >> 6) & 0x3f];
        } else {
            encoded += '=';
        }
        if (i + 2 < size) {
            encoded += table[(temp >> 0) & 0x3f];
        } else {
            encoded += '=';
        }
    }

    return encoded;
}

// username:password -> base64
static std::string authorization_basic(const std::string& username, const std::string& password) {
    auto data = username + ":" + password;
    auto size = data.size();
    auto temp = (uint8_t*)malloc(size);
    memcpy(temp, data.c_str(), size);
    auto encoded = base64_encode(temp, size);
    free(temp);
    return encoded;
}

// hexdump with ascii
static void hexdump(const void* data, size_t size) {
    auto bytes = (const uint8_t*)data;
    for (size_t i = 0; i < size; i += 16) {
        printf("%08lx: ", i);

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

        if (connect(mSocket, (sockaddr*)&addr, sizeof(addr)) < 0) {
            throw std::runtime_error("Failed to connect to host");
        }
    }

    ~Ntrip() { close(mSocket); }

    void authorize(std::string username, std::string password) {
        mUsername = username;
        mPassword = password;
    }

    void request(const std::string& mountpoint) {
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

    void nmea_update(const std::string& nmea) {
        std::string request = nmea + "\r\n";
        if (send(mSocket, request.c_str(), request.size(), 0) < 0) {
            throw std::runtime_error("Failed to send request");
        }
    }

    ssize_t read(void* buffer, size_t size) { return ::read(mSocket, buffer, size); }

private:
    int         mSocket;
    std::string mUsername;
    std::string mPassword;
};

int main(int argc, char** argv) {
    auto  options = parse_configuration(argc, argv);
    auto& host    = options.host;
    auto& output  = options.output;

    // resolve hostname
    auto addr = resolve(host.hostname, host.port);

    // connect to host
    Ntrip ntrip(addr);
    ntrip.authorize(host.username, host.password);

    if (host.mountpoint) {
        ntrip.request(*host.mountpoint);
    } else {
        ntrip.request("");
    }

    if (!host.nmea.empty()) {
        ntrip.nmea_update(host.nmea);
    }

    char temp[4096];
    for (;;) {
        auto bytes = ntrip.read(temp, sizeof(temp));
        if (bytes < 0) {
            throw std::runtime_error("Failed to read from NTRIP");
        } else if (bytes == 0) {
            break;
        }

        hexdump(temp, bytes);

        for (auto& interface : output.interfaces) {
            interface->write(temp, bytes);
        }
    }

    return 0;
}
