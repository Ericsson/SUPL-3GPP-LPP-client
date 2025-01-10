#include "nmea.hpp"

#include <format/nmea/epe.hpp>
#include <format/nmea/gga.hpp>
#include <format/nmea/gst.hpp>
#include <format/nmea/vtg.hpp>
#include <loglet/loglet.hpp>
#include <lpp/location_information.hpp>

#define LOGLET_CURRENT_MODULE "p/ubx"

void NmeaPrint::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    VSCOPE_FUNCTION();
    message->print();
}

void NmeaOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto sentence = message->sentence();
    auto data     = reinterpret_cast<uint8_t const*>(sentence.data());
    auto size     = sentence.size();
    for (auto const& output : mOutput.outputs) {
        if (!output.nmea_support()) continue;
        if (output.print) {
            XINFOF(OUTPUT_PRINT_MODULE, "nmea: %zd bytes", size);
        }

        output.interface->write(data, size);
    }
}

void NmeaLocation::consume(streamline::System& system, DataType&& message) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto gga = dynamic_cast<format::nmea::GgaMessage*>(message.get());
    if (gga) {
        mGga.reset(gga);
        message.release();
    }

    auto vtg = dynamic_cast<format::nmea::VtgMessage*>(message.get());
    if (vtg) {
        mVtg.reset(vtg);
        message.release();
    }

    auto gst = dynamic_cast<format::nmea::GstMessage*>(message.get());
    if (gst) {
        mGst.reset(gst);
        message.release();
    }

    auto epe = dynamic_cast<format::nmea::EpeMessage*>(message.get());
    if (epe) {
        mEpe.reset(epe);
        message.release();
    }

    if (mGga && mVtg && mGst) {
        process(system, *mGga.get(), *mVtg.get(), *mGst.get());
        // NOTE(ewasjon): Should we clear the pointers here?
    } else if (mGga && mVtg && mEpe) {
        process(system, *mGga.get(), *mVtg.get(), *mEpe.get());
        // NOTE(ewasjon): Should we clear the pointers here?
    }
}

NmeaLocation::NmeaLocation(LocationInformationConfig const& config)
    : mGga(nullptr), mVtg(nullptr), mGst(nullptr), mEpe(nullptr), mConfig(config) {}

NmeaLocation::~NmeaLocation() = default;

void NmeaLocation::process(streamline::System& system, format::nmea::GgaMessage const& gga,
                           format::nmea::VtgMessage const& vtg,
                           format::nmea::GstMessage const& gst) {
    process(system, gga, vtg, gst.semi_major(), gst.semi_minor(), gst.orientation(),
            gst.vertical_position_error());
}

void NmeaLocation::process(streamline::System& system, format::nmea::GgaMessage const& gga,
                           format::nmea::VtgMessage const& vtg,
                           format::nmea::EpeMessage const& epe) {
    process(system, gga, vtg, epe.semi_major(), epe.semi_minor(), epe.orientation(),
            epe.vertical_position_error());
}

void NmeaLocation::process(streamline::System& system, format::nmea::GgaMessage const& gga,
                           format::nmea::VtgMessage const& vtg, double semi_major,
                           double semi_minor, double orientation, double vertical_position_error) {
    VSCOPE_FUNCTION();
    if (mConfig.convert_confidence_95_to_68) {
        // 95% confidence to 68% confidence
        // TODO(ewasjon): should this not be 1.95996 (sqrt(3.84)) from 1-degree chi-squared
        // distribution?
        semi_major = semi_major / 2.4477;
        semi_minor = semi_minor / 2.4477;
    }

    auto horizontal_accuracy =
        lpp::HorizontalAccuracy::to_ellipse_39(semi_major, semi_minor, orientation);
    if (mConfig.output_ellipse_68) {
        horizontal_accuracy =
            lpp::HorizontalAccuracy::to_ellipse_68(semi_major, semi_minor, orientation);
    }
    if (mConfig.override_horizontal_confidence >= 0.0 &&
        mConfig.override_horizontal_confidence <= 1.0) {
        horizontal_accuracy.confidence = mConfig.override_horizontal_confidence;
    }

    lpp::LocationInformation location{};
    location.time     = gga.time_of_day();
    location.location = lpp::LocationShape::ha_ellipsoid_altitude_with_uncertainty(
        gga.latitude(), gga.longitude(), gga.altitude(), horizontal_accuracy,
        lpp::VerticalAccuracy::from_1sigma(vertical_position_error));
    location.velocity =
        lpp::VelocityShape::horizontal(vtg.speed_over_ground(), vtg.true_course_over_ground());
    system.push(std::move(location));

    lpp::HaGnssMetrics metrics{};
    metrics.fix_quality          = lpp::FixQuality::INVALID;
    metrics.number_of_satellites = gga.satellites_in_view();
    metrics.hdop                 = gga.h_dop();
    metrics.age_of_corrections   = gga.age_of_differential_corrections();

    switch (gga.fix_quality()) {
    case format::nmea::GgaFixQuality::Invalid:
        metrics.fix_quality = lpp::FixQuality::INVALID;
        break;
    case format::nmea::GgaFixQuality::GpsFix:
        metrics.fix_quality = lpp::FixQuality::STANDALONE;
        break;
    case format::nmea::GgaFixQuality::DgpsFix:
        metrics.fix_quality = lpp::FixQuality::DGPS_FIX;
        break;
    case format::nmea::GgaFixQuality::PpsFix: metrics.fix_quality = lpp::FixQuality::PPS_FIX; break;
    case format::nmea::GgaFixQuality::RtkFixed:
        metrics.fix_quality = lpp::FixQuality::RTK_FIX;
        break;
    case format::nmea::GgaFixQuality::RtkFloat:
        metrics.fix_quality = lpp::FixQuality::RTK_FLOAT;
        break;
    case format::nmea::GgaFixQuality::DeadReckoning:
        metrics.fix_quality = lpp::FixQuality::DEAD_RECKONING;
        break;
    }
    system.push(std::move(metrics));
}
