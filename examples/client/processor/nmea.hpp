#pragma once
#include <memory>

#include <format/nmea/message.hpp>
#include <lpp/location_information.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"

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
    NmeaPrint(PrintConfig const& config) : mConfig(config) {}

    NODISCARD char const* name() const NOEXCEPT override { return "NmeaPrint"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;

private:
    PrintConfig const& mConfig;
};

class NmeaOutput : public streamline::Inspector<NmeaMessage> {
public:
    NmeaOutput(OutputConfig const& output) : mOutput(output) {}

    NODISCARD char const* name() const NOEXCEPT override { return "NmeaOutput"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;

private:
    OutputConfig const& mOutput;
};

class NmeaLocation : public streamline::Consumer<NmeaMessage> {
public:
    NmeaLocation(LocationInformationConfig const& config);
    virtual ~NmeaLocation() override;

    NODISCARD char const* name() const NOEXCEPT override { return "NmeaLocation"; }
    void consume(streamline::System& system, DataType&& message, uint64_t tag) NOEXCEPT override;
    void process(streamline::System& system, format::nmea::GgaMessage const& gga,
                 lpp::VelocityShape velocity, lpp::HorizontalAccuracy horizontal,
                 lpp::VerticalAccuracy vertical);

    NODISCARD lpp::VelocityShape vtg_to_velocity(format::nmea::VtgMessage const& vtg) const;

    NODISCARD lpp::HorizontalAccuracy horizontal_accuracy(double semi_major, double semi_minor,
                                                          double orientation) const;
    NODISCARD lpp::HorizontalAccuracy gst_to_horizontal(format::nmea::GstMessage const& gst) const;
    NODISCARD lpp::HorizontalAccuracy epe_to_horizontal(format::nmea::EpeMessage const& epe) const;

    NODISCARD lpp::VerticalAccuracy gst_to_vertical(format::nmea::GstMessage const& gst) const;
    NODISCARD lpp::VerticalAccuracy epe_to_vertical(format::nmea::EpeMessage const& epe) const;

private:
    std::unique_ptr<format::nmea::GgaMessage> mGga;
    std::unique_ptr<format::nmea::VtgMessage> mVtg;
    std::unique_ptr<format::nmea::GstMessage> mGst;
    std::unique_ptr<format::nmea::EpeMessage> mEpe;

    std::vector<std::string> mOrderReceived;

    LocationInformationConfig const& mConfig;
};
