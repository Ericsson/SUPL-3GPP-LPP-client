#pragma once
#include <format/ubx/message.hpp>

#include <memory>

#include <format/helper/parser.hpp>

namespace format {
namespace ubx {

class Parser : public format::helper::Parser {
public:
    EXPLICIT Parser()          = default;
    virtual ~Parser() override = default;

    NODISCARD virtual char const* name() const NOEXCEPT override;

    NODISCARD std::unique_ptr<Message> try_parse() NOEXCEPT;
    NODISCARD static uint16_t checksum_message(uint8_t* message_data, uint32_t message_length);
    NODISCARD static uint16_t checksum(uint8_t* payload, uint32_t length);

protected:
    NODISCARD bool is_frame_boundary() const NOEXCEPT;
};

}  // namespace ubx
}  // namespace format
