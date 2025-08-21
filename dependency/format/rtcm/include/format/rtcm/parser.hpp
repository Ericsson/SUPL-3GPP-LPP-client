#pragma once
#include <core/core.hpp>
#include <format/helper/parser.hpp>

#include <memory>
#include <vector>

namespace format {
namespace rtcm {

enum class CRCResult {
    OK = 0,
    INVALID_STRING_NOSTAR,
    INVALID_STRING_LENGTH,
    INVALID_VALUE,
};

class Message;
class Parser : public format::helper::Parser {
public:
    EXPLICIT Parser() NOEXCEPT          = default;
    virtual ~Parser() NOEXCEPT override = default;

    NODISCARD virtual char const* name() const NOEXCEPT override;

    NODISCARD std::unique_ptr<Message> try_parse() NOEXCEPT;
    NODISCARD static CRCResult         crc(std::vector<uint8_t> const& buffer);

protected:
    NODISCARD std::string parse_prefix(uint8_t const* data, uint32_t length) const NOEXCEPT;
};

}  // namespace rtcm
}  // namespace format
