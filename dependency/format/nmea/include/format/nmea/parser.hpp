#pragma once
#include <core/core.hpp>
#include <format/helper/parser.hpp>

#include <memory>

namespace format {
namespace nmea {

enum class ChecksumResult {
    OK = 0,
    INVALID_STRING_NOSTAR,
    INVALID_STRING_LENGTH,
    INVALID_VALUE,
};

class Message;
class Parser : public format::helper::Parser {
public:
    EXPLICIT Parser(bool lf_only = false) : mLfOnly(lf_only) {}
    virtual ~Parser() override = default;

    NODISCARD virtual char const* name() const NOEXCEPT override;

    NODISCARD std::unique_ptr<Message> try_parse() NOEXCEPT;
    NODISCARD ChecksumResult checksum(std::string const& buffer) const;

protected:
    NODISCARD std::string parse_prefix(uint8_t const* data, uint32_t length) const NOEXCEPT;

private:
    bool mLfOnly;
};

}  // namespace nmea
}  // namespace format
