#include "location_information.hpp"
#include "client.hpp"

#include <sstream>

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

LOGLET_MODULE2(p, li);
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
    result << "\"semi-major\": " << horizontal_accuracy.semi_major;
    result << ",\"semi-minor\": " << horizontal_accuracy.semi_minor;
    result << ",\"orientation\": " << horizontal_accuracy.orientation;
    result << ",\"confidence\": " << horizontal_accuracy.confidence;
}

static void vertical_accuracy_json(std::stringstream&           result,
                                   lpp::VerticalAccuracy const& vertical_accuracy) {
    result << "\"uncertainty\": " << vertical_accuracy.uncertainty;
    result << ",\"confidence\": " << vertical_accuracy.confidence;
}

void LocationOutput::inspect(streamline::System&, DataType const& location, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();

    std::stringstream result;
    result << "{";
    result << "\"type\": \"location\",";
    result << ",\"time\": \"" << ts::Utc{location.time}.rfc3339() << "\"";
    if (location.location.has_value()) {
        auto const& shape = location.location.const_value();
        result << ",\"location\": {";
        switch (shape.kind) {
        case lpp::LocationShape::Kind::HighAccuracyEllipsoidPointWithUncertaintyEllipse:
            result << "\"type\": \"high-accuracy-ellipsoid-point-with-uncertainty-ellipse\"";
            result << ",\"latitude\": " << shape.data.haepue.latitude;
            result << ",\"longitude\": " << shape.data.haepue.longitude;
            result << ",\"horizontal-accuracy\": ";
            horizontal_accuracy_json(result, shape.data.haepue.horizontal_accuracy);
            break;
        case lpp::LocationShape::Kind::
            HighAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid:
            result << "\"type\": "
                      "\"high-accuracy-ellipsoid-point-with-altitude-and-uncertainty-ellipsoid\"";
            result << ",\"latitude\": " << shape.data.haepaue.latitude;
            result << ",\"longitude\": " << shape.data.haepaue.longitude;
            result << ",\"altitude\": " << shape.data.haepaue.altitude;
            result << ",\"horizontal-accuracy\": ";
            horizontal_accuracy_json(result, shape.data.haepaue.horizontal_accuracy);
            result << ",\"vertical-accuracy\": ";
            vertical_accuracy_json(result, shape.data.haepaue.vertical_accuracy);
            break;
        default: result << "\"type\": \"unknown\""; break;
        }

        result << "}";
    }
    if (location.velocity.has_value()) {
        result << ",\"velocity\": {";
        result << "}";
    }
    result << "}\n";
    auto sentence = result.str();
    auto data     = reinterpret_cast<uint8_t const*>(sentence.data());
    auto size     = sentence.size();
    for (auto const& output : mOutput.outputs) {
        if (!output.location_support()) continue;
        if (!output.accept_tag(tag)) {
            XDEBUGF(OUTPUT_PRINT_MODULE, "location: tag %llX not accepted", tag);
            continue;
        }
        if (output.print) {
            XINFOF(OUTPUT_PRINT_MODULE, "location: %zd bytes", size);
        }

        output.interface->write(data, size);
    }
}
