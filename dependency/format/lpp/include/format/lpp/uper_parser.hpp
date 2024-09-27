#pragma once
#include <core/core.hpp>

#include <memory>

#include <format/helper/parser.hpp>

struct LPP_Message;

namespace format {
namespace lpp {

class UperParser : public format::helper::Parser {
public:
    EXPLICIT UperParser() NOEXCEPT  = default;
    ~UperParser() NOEXCEPT override = default;

    NODISCARD virtual char const* name() const NOEXCEPT override;
    NODISCARD LPP_Message*        try_parse() NOEXCEPT;
};

}  // namespace lpp
}  // namespace format
