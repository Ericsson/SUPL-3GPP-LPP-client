#pragma once
#include <core/core.hpp>

#include <memory>

#include <format/helper/parser.hpp>

struct LPP_Message;
struct A_GNSS_ProvideAssistanceData;

namespace format {
namespace lpp {

class UperParser : public format::helper::Parser {
public:
    EXPLICIT UperParser()  = default;
    ~UperParser() override = default;

    NODISCARD virtual char const*           name() const NOEXCEPT override;
    NODISCARD LPP_Message*                  try_parse() NOEXCEPT;
    NODISCARD A_GNSS_ProvideAssistanceData* try_parse_provide_assistance_data() NOEXCEPT;
};

}  // namespace lpp
}  // namespace format
