#pragma once
#include <memory>

#include <format/ctrl/message.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "options.hpp"

namespace format {
namespace ctrl {
class CellId;
class IdentityImsi;
}  // namespace ctrl
}  // namespace format

using CtrlMessage = std::unique_ptr<format::ctrl::Message>;

namespace streamline {
template <>
struct Clone<CtrlMessage> {
    CtrlMessage operator()(CtrlMessage const& value) { return value->clone(); }
};
}  // namespace streamline

class CtrlPrint : public streamline::Inspector<CtrlMessage> {
public:
    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;
};

class CtrlOutput : public streamline::Inspector<CtrlMessage> {
public:
    CtrlOutput(OutputOptions const& options) : mOptions(options) {}

    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;

private:
    OutputOptions const& mOptions;
};

class CtrlEvents : public streamline::Inspector<CtrlMessage> {
public:
    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;

    std::function<void(format::ctrl::CellId const&)>       on_cell_id;
    std::function<void(format::ctrl::IdentityImsi const&)> on_identity_imsi;
};
