#include <LPP-Message.h>
#include <generator/spartn/generator.h>
#include <generator/spartn/transmitter.h>
#include <generator/spartn2/generator.hpp>
#include <unistd.h>
#include "options.hpp"

struct StdinStream {
    StdinStream() {
        fd = fileno(stdin);
        if (fd < 0) throw std::runtime_error("Failed to open stdin");
    }

    ~StdinStream() { close(fd); }

    // Read from stdin all fill the buffer
    bool read() {
        auto    bytes_read = 0;
        uint8_t temp[1024];
        for (;;) {
            // check if there is data to be read
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);
            timeval timeout{0, 0};
            auto    ret = select(fd + 1, &readfds, NULL, NULL, &timeout);
            if (ret < 0) {
                throw std::runtime_error("Failed to read from stdin");
            } else if (ret == 0) {
                // no data to be read
                break;
            }

            // read data
            auto bytes = ::read(fd, temp, sizeof(temp));
            if (bytes < 0) {
                throw std::runtime_error("Failed to read from stdin");
            } else if (bytes == 0) {
                break;
            }

            // append to buffer
            buffer.insert(buffer.end(), temp, temp + bytes);
            bytes_read += bytes;
        }

        return bytes_read > 0;
    }

    void clear(size_t bytes) { buffer.erase(buffer.begin(), buffer.begin() + bytes); }

    int                  fd;
    std::vector<uint8_t> buffer;
};

#define ALLOC_ZERO(T) ((T*)calloc(1, sizeof(T)))
LPP_Message* lpp_decode(StdinStream& stream) {
    asn_codec_ctx_t stack_ctx{};
    stack_ctx.max_stack_size = 1024 * 1024 * 4;

    LPP_Message*   lpp  = ALLOC_ZERO(LPP_Message);
    asn_dec_rval_t rval = uper_decode_complete(&stack_ctx, &asn_DEF_LPP_Message, (void**)&lpp,
                                               stream.buffer.data(), stream.buffer.size());
    if (rval.code != RC_OK) {
        free(lpp);
        return NULL;
    }

    stream.clear(rval.consumed);
    return lpp;
}

LPP_Message* next_message(StdinStream& stream) {
    if (stream.buffer.empty()) {
        if (!stream.read()) return NULL;
    }

    if (stream.buffer.size() > 0) {
        auto message = lpp_decode(stream);
        if (message) return message;
        if (!stream.read()) return NULL;
    }

    return NULL;
}

static void hexdump(const uint8_t* data, size_t size) {
    // hexdump with ascii
    constexpr auto width = 16;
    for (size_t i = 0; i < size; i += width) {
        printf("%04zx: ", i);
        for (size_t j = 0; j < width; j++) {
            if (i + j < size) {
                printf("%02x ", data[i + j]);
            } else {
                printf("   ");
            }
        }
        printf(" ");
        for (size_t j = 0; j < width; j++) {
            if (i + j < size) {
                auto c = data[i + j];
                if (c >= 0x20 && c <= 0x7E) {
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

int main(int argc, char** argv) {
    auto  options = parse_configuration(argc, argv);
    auto& output  = options.output;

    StdinStream      stream{};
    SPARTN_Generator generator{};

    generator::spartn::Generator spartn2_generator{};

    for (;;) {
        auto message = next_message(stream);
        if (!message) break;

        auto gUraOverride          = 2;
        auto gUBloxClockCorrection = true;
        auto gForceIodeContinuity  = true;

#if 1
        auto messages2 = spartn2_generator.generate(message);

        for (auto& msg : messages2) {
            printf("Message: %d-%d\n", msg.message_type(), msg.message_subtype());
            auto data = msg.build();
            hexdump(data.data(), data.size());

            for (auto& interface : output.interfaces) {
                interface->write(data.data(), data.size());
            }
        }
#else
        auto messages =
            generator.generate(message, gUraOverride, gUBloxClockCorrection, gForceIodeContinuity);

        printf("Generated %zu messages\n", messages.size());

        for (auto& msg : messages) {
            auto bytes = SPARTN_Transmitter::build(msg);
            for (auto& interface : output.interfaces) {
                interface->write(bytes.data(), bytes.size());
            }
        }
#endif
        ASN_STRUCT_FREE(asn_DEF_LPP_Message, message);
    }

    return 0;
}
