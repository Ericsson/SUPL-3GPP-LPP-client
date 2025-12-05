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
            XVERBOSEF(OUTPUT_PRINT_MODULE, "location: tag %llX not accepted", tag);
            continue;
        }
        XDEBUGF(OUTPUT_PRINT_MODULE, "location: (%zd bytes) tag=%llX", size, tag);

        ASSERT(output.stage, "stage is null");
        output.stage->write(OUTPUT_FORMAT_LOCATION, data, size);
    }
}
