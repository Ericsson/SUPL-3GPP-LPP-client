#pragma once
#include <core/core.hpp>
#include <format/helper/parser.hpp>

#include <memory>
#include <string>
#include <vector>

namespace format {
namespace ctrl {

class Message;
class Parser : public format::helper::Parser {
public:
    EXPLICIT Parser()          = default;
    virtual ~Parser() override = default;

    NODISCARD virtual char const* name() const NOEXCEPT override;
    NODISCARD std::unique_ptr<Message> try_parse() NOEXCEPT;

protected:
    NODISCARD std::unique_ptr<Message> parse_cid(std::string const&              message,
                                                 std::vector<std::string> const& tokens) NOEXCEPT;
    NODISCARD                          std::unique_ptr<Message>
    parse_identity(std::string const& message, std::vector<std::string> const& tokens) NOEXCEPT;
};

}  // namespace ctrl
}  // namespace format
