#include "options.hpp"

#include <sstream>
#include <unistd.h>

#ifdef INCLUDE_GENERATOR_SPARTN_OLD
#include <generator/spartn/generator.h>
#include <generator/spartn/transmitter.h>
#endif

#ifdef INCLUDE_GENERATOR_SPARTN
#include <generator/spartn2/generator.hpp>
#endif

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
            auto    ret = select(fd + 1, &readfds, nullptr, nullptr, &timeout);
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

    void clear(size_t bytes) {
        buffer.erase(buffer.begin(), buffer.begin() + static_cast<long>(bytes));
    }

    int                  fd;
    std::vector<uint8_t> buffer;
};

#define ALLOC_ZERO(T) (reinterpret_cast<T*>(calloc(1, sizeof(T))))
LPP_Message* lpp_decode(StdinStream& stream) {
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

LPP_Message* next_message(StdinStream& stream) {
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

int main(int argc, char** argv) {
    auto  options = parse_configuration(argc, argv);
    auto& output  = options.output;
    auto& spartn  = options.spartn;

    StdinStream stream{};

#ifdef INCLUDE_GENERATOR_SPARTN_OLD
    SPARTN_Generator old_generator{};
#endif

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

        if (options.format == Format::XER) {
            std::stringstream buffer;
            xer_encode(
                &asn_DEF_LPP_Message, message, XER_F_BASIC,
                [](void const* text_buffer, size_t text_size, void* app_key) -> int {
                    auto string_stream = static_cast<std::ostream*>(app_key);
                    string_stream->write(static_cast<char const*>(text_buffer),
                                         static_cast<std::streamsize>(text_size));
                    return 0;
                },
                &buffer);
            auto xer_message = buffer.str();
            for (auto& interface : output.interfaces) {
                interface->write(xer_message.data(), xer_message.size());
            }
        }
#ifdef INCLUDE_GENERATOR_SPARTN
        else if (options.format == Format::SPARTN_NEW) {
            auto messages2 = new_generator.generate(message);
            for (auto& msg : messages2) {
                auto data = msg.build();
                for (auto& interface : output.interfaces) {
                    interface->write(data.data(), data.size());
                }
            }
        }
#endif
#ifdef INCLUDE_GENERATOR_SPARTN_OLD
        else if (options.format == Format::SPARTN_OLD) {
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
#endif

        ASN_STRUCT_FREE(asn_DEF_LPP_Message, message);
    }

    return 0;
}
