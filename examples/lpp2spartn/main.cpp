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

int main(int argc, char** argv) {
    auto  options = parse_configuration(argc, argv);
    auto& output  = options.output;
    auto& spartn  = options.spartn;

    StdinStream      stream{};
    SPARTN_Generator old_generator{};

    auto gUraOverride          = 2;
    auto gUBloxClockCorrection = true;
    auto gForceIodeContinuity  = true;

    generator::spartn::Generator new_generator{};

    new_generator.set_ura_override(gUraOverride);
    new_generator.set_ublox_clock_correction(gUBloxClockCorrection);
    if (gForceIodeContinuity) {
        new_generator.set_continuity_indicator(320.0);
    }

    if (spartn.iode_shift) {
        new_generator.set_iode_shift(true);
    } else {
        new_generator.set_iode_shift(false);
    }

    for (;;) {
        auto message = next_message(stream);
        if (!message) break;

        if (options.format == Format::SPARTN_NEW) {
            auto messages2 = new_generator.generate(message);
            for (auto& msg : messages2) {
                auto data = msg.build();
                for (auto& interface : output.interfaces) {
                    interface->write(data.data(), data.size());
                }
            }
        } else if (options.format == Format::SPARTN_OLD) {
            auto messages = old_generator.generate(message, gUraOverride, gUBloxClockCorrection,
                                                   gForceIodeContinuity);

            // NOTE(ewasjon): This looks stupid, and it is, but for testing purposes we want to
            // output the messages in a specific order. Will be fixed in the future.
            for (auto& msg : messages) {
                if (msg->message_type == 2 && msg->message_sub_type == 0) {
                    auto bytes = SPARTN_Transmitter::build(msg);
                    for (auto& interface : output.interfaces) {
                        interface->write(bytes.data(), bytes.size());
                    }
                }
            }

            for (auto& msg : messages) {
                if (msg->message_type == 0 && msg->message_sub_type == 0) {
                    auto bytes = SPARTN_Transmitter::build(msg);
                    for (auto& interface : output.interfaces) {
                        interface->write(bytes.data(), bytes.size());
                    }
                }
            }

            for (auto& msg : messages) {
                if (msg->message_type == 0 && msg->message_sub_type == 1) {
                    auto bytes = SPARTN_Transmitter::build(msg);
                    for (auto& interface : output.interfaces) {
                        interface->write(bytes.data(), bytes.size());
                    }
                }
            }

            for (auto& msg : messages) {
                if (msg->message_type == 0 && msg->message_sub_type == 2) {
                    auto bytes = SPARTN_Transmitter::build(msg);
                    for (auto& interface : output.interfaces) {
                        interface->write(bytes.data(), bytes.size());
                    }
                }
            }
            for (auto& msg : messages) {
                if (msg->message_type == 1 && msg->message_sub_type == 0) {
                    auto bytes = SPARTN_Transmitter::build(msg);
                    for (auto& interface : output.interfaces) {
                        interface->write(bytes.data(), bytes.size());
                    }
                }
            }
            for (auto& msg : messages) {
                if (msg->message_type == 1 && msg->message_sub_type == 1) {
                    auto bytes = SPARTN_Transmitter::build(msg);
                    for (auto& interface : output.interfaces) {
                        interface->write(bytes.data(), bytes.size());
                    }
                }
            }
            for (auto& msg : messages) {
                if (msg->message_type == 1 && msg->message_sub_type == 2) {
                    auto bytes = SPARTN_Transmitter::build(msg);
                    for (auto& interface : output.interfaces) {
                        interface->write(bytes.data(), bytes.size());
                    }
                }
            }
        }

        ASN_STRUCT_FREE(asn_DEF_LPP_Message, message);
    }

    return 0;
}
