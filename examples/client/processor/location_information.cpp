#include "location_information.hpp"
#include "client.hpp"

#include <iomanip>
#include <sstream>

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

LOGLET_MODULE2(p, li);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, li)

void LocationCollector::inspect(streamline::System&, DataType const& location, uint64_t) NOEXCEPT {
    VSCOPE_FUNCTION();
    mProgram.update_location_information(location);
}

void MetricsCollector::inspect(streamline::System&, DataType const& metrics, uint64_t) NOEXCEPT {
    VSCOPE_FUNCTION();
    mProgram.update_gnss_metrics(metrics);
}

static void horizontal_accuracy_json(std::stringstream&             result,
                                     lpp::HorizontalAccuracy const& horizontal_accuracy) {
    result << "{";
    result << "\"semi-major\": " << std::setprecision(15) << horizontal_accuracy.semi_major;
    result << ",\"semi-minor\": " << std::setprecision(15) << horizontal_accuracy.semi_minor;
    result << ",\"orientation\": " << std::setprecision(15) << horizontal_accuracy.orientation;
    result << ",\"confidence\": " << std::setprecision(15) << horizontal_accuracy.confidence;
    result << "}";
}

static void vertical_accuracy_json(std::stringstream&           result,
                                   lpp::VerticalAccuracy const& vertical_accuracy) {
    result << "{";
    result << "\"uncertainty\": " << std::setprecision(15) << vertical_accuracy.uncertainty;
    result << ",\"confidence\": " << std::setprecision(15) << vertical_accuracy.confidence;
    result << "}";
}

static char const* fix_quality_json(lpp::FixQuality fix_quality) {
    switch (fix_quality) {
    case lpp::FixQuality::INVALID: return "invalid";
    case lpp::FixQuality::STANDALONE: return "standalone";
    case lpp::FixQuality::DgpsFix: return "dgps";
    case lpp::FixQuality::PpsFix: return "pps";
    case lpp::FixQuality::RtkFix: return "rtk-fixed";
    case lpp::FixQuality::RtkFloat: return "rtk-float";
    case lpp::FixQuality::DeadReckoning: return "dead-reckoning";
    case lpp::FixQuality::ManualInput: return "manual-input";
    case lpp::FixQuality::SIMULATION: return "simulation";
    case lpp::FixQuality::WaasFix: return "waas";
    }
    return "unknown";
}

static void gnss_metrics_json(std::stringstream& result, lpp::HaGnssMetrics const& metrics) {
    result << "{";
    result << "\"fix-type\": \"" << fix_quality_json(metrics.fix_quality) << "\"";
    result << ",\"satellites\": " << metrics.number_of_satellites;
    if (metrics.age_of_corrections.has_value()) {
        result << ",\"age-of-corrections\": " << std::setprecision(15)
               << metrics.age_of_corrections.const_value();
    }
    if (metrics.hdop.has_value()) {
        result << ",\"hdop\": " << std::setprecision(15) << metrics.hdop.const_value();
    }
    if (metrics.pdop.has_value()) {
        result << ",\"pdop\": " << std::setprecision(15) << metrics.pdop.const_value();
    }
    result << "}";
}

static void velocity_json(std::stringstream& result, lpp::VelocityShape const& shape) {
    result << "{";
    switch (shape.kind) {
    case lpp::VelocityShape::Kind::HorizontalVelocity:
        result << "\"type\": \"horizontal\"";
        result << ",\"speed\": " << std::setprecision(15) << shape.data.hv.horizontal.speed;
        result << ",\"bearing\": " << std::setprecision(15) << shape.data.hv.horizontal.bearing;
        break;
    case lpp::VelocityShape::Kind::HorizontalWithVerticalVelocity:
        result << "\"type\": \"horizontal-with-vertical\"";
        result << ",\"speed\": " << std::setprecision(15) << shape.data.hvv.horizontal.speed;
        result << ",\"bearing\": " << std::setprecision(15) << shape.data.hvv.horizontal.bearing;
        result << ",\"vertical-speed\": " << std::setprecision(15) << shape.data.hvv.vertical.speed;
        result << ",\"vertical-direction\": \""
               << (shape.data.hvv.vertical.direction == lpp::VerticalDirection::Up ? "up" : "down")
               << "\"";
        break;
    case lpp::VelocityShape::Kind::HorizontalVelocityWithUncertainty:
        result << "\"type\": \"horizontal-with-uncertainty\"";
        result << ",\"speed\": " << std::setprecision(15) << shape.data.hvu.horizontal.speed;
        result << ",\"speed-uncertainty\": " << std::setprecision(15)
               << shape.data.hvu.horizontal.uncertainty;
        result << ",\"bearing\": " << std::setprecision(15) << shape.data.hvu.horizontal.bearing;
        break;
    case lpp::VelocityShape::Kind::HorizontalWithVerticalVelocityAndUncertainty:
        result << "\"type\": \"horizontal-with-vertical-and-uncertainty\"";
        result << ",\"speed\": " << std::setprecision(15) << shape.data.hvvu.horizontal.speed;
        result << ",\"speed-uncertainty\": " << std::setprecision(15)
               << shape.data.hvvu.horizontal.uncertainty;
        result << ",\"bearing\": " << std::setprecision(15) << shape.data.hvvu.horizontal.bearing;
        result << ",\"vertical-speed\": " << std::setprecision(15)
               << shape.data.hvvu.vertical.speed;
        result << ",\"vertical-speed-uncertainty\": " << std::setprecision(15)
               << shape.data.hvvu.vertical.uncertainty;
        result << ",\"vertical-direction\": \""
               << (shape.data.hvvu.vertical.direction == lpp::VerticalDirection::Up ? "up" : "down")
               << "\"";
        break;
    }
    result << "}";
}

void LocationOutput::inspect(streamline::System&, DataType const& location, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();

    std::stringstream result;
    result << "{";
    result << "\"type\": \"location\"";
    result << ",\"time\": \"" << ts::Utc{location.time}.rfc3339() << "\"";
    if (location.location.has_value()) {
        auto const& shape = location.location.const_value();
        result << ",\"location\": {";
        switch (shape.kind) {
        case lpp::LocationShape::Kind::HighAccuracyEllipsoidPointWithUncertaintyEllipse:
            result << "\"type\": \"high-accuracy-ellipsoid-point-with-uncertainty-ellipse\"";
            result << ",\"latitude\": " << std::setprecision(15) << shape.data.haepue.latitude;
            result << ",\"longitude\": " << std::setprecision(15) << shape.data.haepue.longitude;
            result << ",\"horizontal-accuracy\": ";
            horizontal_accuracy_json(result, shape.data.haepue.horizontal_accuracy);
            break;
        case lpp::LocationShape::Kind::
            HighAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid:
            result << "\"type\": "
                      "\"high-accuracy-ellipsoid-point-with-altitude-and-uncertainty-ellipsoid\"";
            result << ",\"latitude\": " << std::setprecision(15) << shape.data.haepaue.latitude;
            result << ",\"longitude\": " << std::setprecision(15) << shape.data.haepaue.longitude;
            result << ",\"altitude\": " << std::setprecision(15) << shape.data.haepaue.altitude;
            result << ",\"horizontal-accuracy\": ";
            horizontal_accuracy_json(result, shape.data.haepaue.horizontal_accuracy);
            result << ",\"vertical-accuracy\": ";
            vertical_accuracy_json(result, shape.data.haepaue.vertical_accuracy);
            break;
        }

        result << "}";
    }
    if (location.velocity.has_value()) {
        result << ",\"velocity\": ";
        velocity_json(result, location.velocity.const_value());
    }
    if (mProgram.latest_gnss_metrics.has_value()) {
        result << ",\"gnss-metrics\": ";
        gnss_metrics_json(result, mProgram.latest_gnss_metrics.const_value());
    }
    result << "}\n";
    auto sentence = result.str();
    auto data     = reinterpret_cast<uint8_t const*>(sentence.data());
    auto size     = sentence.size();
    for (auto const& output : mProgram.output.outputs) {
        if (!output.location_support()) continue;
        if (!output.accept_tag(tag)) {
            XVERBOSEF(OUTPUT_PRINT_MODULE, "location: tag %s not accepted: %s",
                      output.tag_name(tag).c_str(), output.reject_reason(tag).c_str());
            continue;
        }
        XDEBUGF(OUTPUT_PRINT_MODULE, "location: (%zd bytes) tag=%llX", size, tag);

        ASSERT(output.stage, "stage is null");
        output.stage->write(OUTPUT_FORMAT_LOCATION, data, size);
    }
}
