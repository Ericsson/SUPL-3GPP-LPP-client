#pragma once
#include <memory>

#include <format/nmea/message.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "options.hpp"

namespace format {
namespace nmea {
class GgaMessage;
class VtgMessage;
class GstMessage;
class EpeMessage;
}  // namespace nmea
}  // namespace format

using NmeaMessage = std::unique_ptr<format::nmea::Message>;

namespace streamline {
template <>
struct Clone<NmeaMessage> {
    NmeaMessage operator()(NmeaMessage const& value) { return value->clone(); }
};
}  // namespace streamline

class NmeaPrint : public streamline::Inspector<NmeaMessage> {
public:
    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;
};

class NmeaOutput : public streamline::Inspector<NmeaMessage> {
public:
    NmeaOutput(OutputOptions const& options) : mOptions(options) {}

    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;

private:
    OutputOptions const& mOptions;
};

class NmeaLocation : public streamline::Consumer<NmeaMessage> {
public:
    NmeaLocation(bool convert_confidence95_to_68, double override_horizontal_confidence,
                 bool output_ellipse_68);
    virtual ~NmeaLocation() override;

    void consume(streamline::System& system, DataType&& message) NOEXCEPT override;
    void process(streamline::System& system, format::nmea::GgaMessage const& gga,
                 format::nmea::VtgMessage const& vtg, format::nmea::GstMessage const& gst);
    void process(streamline::System& system, format::nmea::GgaMessage const& gga,
                 format::nmea::VtgMessage const& vtg, format::nmea::EpeMessage const& epe);
    void process(streamline::System& system, format::nmea::GgaMessage const& gga,
                 format::nmea::VtgMessage const& vtg, double semi_major, double semi_minor,
                 double orientation, double vertical_position_error);

private:
    bool   mConvertConfidence95To68;
    double mOverrideHorizontalConfidence;
    bool   mOutputEllipse68;

    std::unique_ptr<format::nmea::GgaMessage> mGga;
    std::unique_ptr<format::nmea::VtgMessage> mVtg;
    std::unique_ptr<format::nmea::GstMessage> mGst;
    std::unique_ptr<format::nmea::EpeMessage> mEpe;
};
