#pragma once
#include <memory>

#include <format/ctrl/message.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"

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
    char const* name() const NOEXCEPT override { return "CtrlPrint"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;
};

class CtrlOutput : public streamline::Inspector<CtrlMessage> {
public:
    CtrlOutput(OutputConfig const& config) : mConfig(config) {}

    char const* name() const NOEXCEPT override { return "CtrlOutput"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;

private:
    OutputConfig const& mConfig;
};

class CtrlEvents : public streamline::Inspector<CtrlMessage> {
public:
    char const* name() const NOEXCEPT override { return "CtrlEvents"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;

    std::function<void(format::ctrl::CellId const&)>       on_cell_id;
    std::function<void(format::ctrl::IdentityImsi const&)> on_identity_imsi;
};
