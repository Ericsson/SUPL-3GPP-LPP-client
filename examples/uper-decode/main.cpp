#include "options.hpp"

#include <sstream>
#include <unistd.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <LPP-Message.h>
#pragma GCC diagnostic pop

struct StdinStream {
    StdinStream() {
        fd = fileno(stdin);
        if (fd < 0) throw std::runtime_error("Failed to open stdin");
    }

    ~StdinStream() { close(fd); }

    bool read() {
        auto    bytes_read = 0;
        uint8_t temp[1024];
        for (;;) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);
            timeval timeout{0, 0};
            auto    ret = select(fd + 1, &readfds, nullptr, nullptr, &timeout);
            if (ret < 0) {
                throw std::runtime_error("Failed to read from stdin");
            } else if (ret == 0) {
                break;
            }

            auto bytes = ::read(fd, temp, sizeof(temp));
            if (bytes < 0) {
                throw std::runtime_error("Failed to read from stdin");
            } else if (bytes == 0) {
                break;
            }

            buffer.insert(buffer.end(), temp, temp + bytes);
            bytes_read += bytes;
        }

        return bytes_read > 0;
    }

    void clear(size_t bytes) {
        buffer.erase(buffer.begin(), buffer.begin() + static_cast<long>(bytes));
    }

    int                  fd;
    std::vector<uint8_t> buffer;
};

#define ALLOC_ZERO(T) (reinterpret_cast<T*>(calloc(1, sizeof(T))))
static LPP_Message* lpp_decode(StdinStream& stream) {
    asn_codec_ctx_t stack_ctx{};
    stack_ctx.max_stack_size = 1024 * 1024 * 4;

    LPP_Message*   lpp = ALLOC_ZERO(LPP_Message);
    asn_dec_rval_t rval =
        uper_decode_complete(&stack_ctx, &asn_DEF_LPP_Message, reinterpret_cast<void**>(&lpp),
                             stream.buffer.data(), stream.buffer.size());
    if (rval.code != RC_OK) {
        free(lpp);
        return nullptr;
    }

    stream.clear(rval.consumed);
    return lpp;
}

static LPP_Message* next_message(StdinStream& stream) {
    if (stream.buffer.empty()) {
        if (!stream.read()) return nullptr;
    }

    if (stream.buffer.size() > 0) {
        auto message = lpp_decode(stream);
        if (message) return message;
        if (!stream.read()) return nullptr;
    }

    return nullptr;
}

static void print_xer(LPP_Message* message) {
    xer_fprint(stdout, &asn_DEF_LPP_Message, message);
}

int main(int argc, char** argv) {
    auto options = parse_configuration(argc, argv);

    StdinStream stream{};

    for (;;) {
        auto message = next_message(stream);
        if (!message) break;
        print_xer(message);
        ASN_STRUCT_FREE(asn_DEF_LPP_Message, message);
    }

    return 0;
}
