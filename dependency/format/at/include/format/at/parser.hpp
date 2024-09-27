#pragma once
#include <core/core.hpp>
#include <format/helper/parser.hpp>

#include <memory>
#include <string>
#include <vector>

namespace format {
namespace at {

class Message;
class Response;
class Parser : public format::helper::Parser {
public:
    EXPLICIT Parser() NOEXCEPT          = default;
    virtual ~Parser() NOEXCEPT override = default;

    NODISCARD virtual char const* name() const NOEXCEPT override;

    void process() NOEXCEPT;

    NODISCARD bool   has_lines() const NOEXCEPT { return !mLines.empty(); }
    NODISCARD size_t count() const NOEXCEPT { return mLines.size(); }
    NODISCARD std::string skip_line() NOEXCEPT;
    NODISCARD std::string const& peek_line() const NOEXCEPT;

private:
    NODISCARD bool process_line(std::string& line) NOEXCEPT;

    std::vector<std::string> mLines;
};

}  // namespace at
}  // namespace format
