#pragma once
#include <memory>

#include <format/ubx/message.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"

namespace format {
namespace ubx {
class UbxNavPvt;
}
}  // namespace format

using UbxMessage = std::unique_ptr<format::ubx::Message>;

namespace streamline {
template <>
struct Clone<UbxMessage> {
    UbxMessage operator()(UbxMessage const& value) { return value->clone(); }
};
}  // namespace streamline

class UbxPrint : public streamline::Inspector<UbxMessage> {
public:
    const char* name() const NOEXCEPT override { return "UbxPrint"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;
};

class UbxOutput : public streamline::Inspector<UbxMessage> {
public:
    UbxOutput(OutputConfig const& output) : mOutput(output) {}

    const char* name() const NOEXCEPT override { return "UbxOutput"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;

private:
    OutputConfig const& mOutput;
};

class UbxLocation : public streamline::Inspector<UbxMessage> {
public:
    UbxLocation(LocationInformationConfig const& config) : mConfig(config) {}

    const char* name() const NOEXCEPT override { return "UbxLocation"; }
    void inspect(streamline::System& system, DataType const& message, uint64_t tag) NOEXCEPT override;
    void nav_pvt(streamline::System& system, format::ubx::UbxNavPvt const& nav_pvt);

private:
    LocationInformationConfig const& mConfig;
};
