#pragma once
#include <memory>

#include <format/ubx/message.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "options.hpp"

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
    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;
};

class UbxOutput : public streamline::Inspector<UbxMessage> {
public:
    UbxOutput(OutputOptions const& options) : mOptions(options) {}

    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;

private:
    OutputOptions const& mOptions;
};

class UbxLocation : public streamline::Inspector<UbxMessage> {
public:
    UbxLocation(bool convert_confidence95_to_68, double override_horizontal_confidence,
                bool output_ellipse_68)
        : mConvertConfidence95To68(convert_confidence95_to_68),
          mOverrideHorizontalConfidence(override_horizontal_confidence),
          mOutputEllipse68(output_ellipse_68) {}

    void inspect(streamline::System& system, DataType const& message) NOEXCEPT override;
    void nav_pvt(streamline::System& system, format::ubx::UbxNavPvt const& nav_pvt);

private:
    bool   mConvertConfidence95To68;
    double mOverrideHorizontalConfidence;
    bool   mOutputEllipse68;
};
