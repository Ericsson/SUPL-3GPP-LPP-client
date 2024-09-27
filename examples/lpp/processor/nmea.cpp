#include "nmea.hpp"

#include <format/nmea/epe.hpp>
#include <format/nmea/gga.hpp>
#include <format/nmea/gst.hpp>
#include <format/nmea/vtg.hpp>
#include <lpp/location_information.h>

void NmeaPrint::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    message->print();
}

void NmeaOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    auto sentence = message->sentence();
    auto data     = reinterpret_cast<uint8_t const*>(sentence.data());
    auto size     = sentence.size();
    for (auto const& output : mOptions.outputs) {
        if ((output.format & OUTPUT_FORMAT_NMEA) != 0) {
            output.interface->write(data, size);
        }
    }
}

void NmeaLocation::consume(streamline::System& system, DataType&& message) NOEXCEPT {
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

NmeaLocation::NmeaLocation(bool convert_confidence95_to_68, double override_horizontal_confidence,
                           bool output_ellipse_68)
    : mConvertConfidence95To68(convert_confidence95_to_68),
      mOverrideHorizontalConfidence(override_horizontal_confidence),
      mOutputEllipse68(output_ellipse_68), mGga(nullptr), mVtg(nullptr), mGst(nullptr),
      mEpe(nullptr) {}

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
    using namespace location_information;

    if (mConvertConfidence95To68) {
        // 95% confidence to 68% confidence
        // TODO(ewasjon): should this not be 1.95996 (sqrt(3.84)) from 1-degree chi-squared
        // distribution?
        semi_major = semi_major / 2.4477;
        semi_minor = semi_minor / 2.4477;
    }

    auto horizontal_accuracy =
        HorizontalAccuracy::to_ellipse_39(semi_major, semi_minor, orientation);
    if (mOutputEllipse68) {
        horizontal_accuracy =
            HorizontalAccuracy::to_ellipse_68(semi_major, semi_minor, orientation);
    }
    if (mOverrideHorizontalConfidence >= 0.0) {
        horizontal_accuracy.confidence = mOverrideHorizontalConfidence;
    }

    LocationInformation location{};
    location.time     = gga.time_of_day();
    location.location = LocationShape::ha_ellipsoid_altitude_with_uncertainty(
        gga.latitude(), gga.longitude(), gga.altitude(), horizontal_accuracy,
        VerticalAccuracy::from_1sigma(vertical_position_error));
    location.velocity =
        VelocityShape::horizontal(vtg.speed_over_ground(), vtg.true_course_over_ground());
    system.push(std::move(location));

    HaGnssMetrics metrics{};
    metrics.fix_quality          = FixQuality::INVALID;
    metrics.number_of_satellites = gga.satellites_in_view();
    metrics.hdop                 = gga.h_dop();
    metrics.age_of_corrections   = gga.age_of_differential_corrections();

    switch (gga.fix_quality()) {
    case format::nmea::GgaFixQuality::Invalid: metrics.fix_quality = FixQuality::INVALID; break;
    case format::nmea::GgaFixQuality::GpsFix: metrics.fix_quality = FixQuality::STANDALONE; break;
    case format::nmea::GgaFixQuality::DgpsFix: metrics.fix_quality = FixQuality::DGPS_FIX; break;
    case format::nmea::GgaFixQuality::PpsFix: metrics.fix_quality = FixQuality::PPS_FIX; break;
    case format::nmea::GgaFixQuality::RtkFixed: metrics.fix_quality = FixQuality::RTK_FIX; break;
    case format::nmea::GgaFixQuality::RtkFloat: metrics.fix_quality = FixQuality::RTK_FLOAT; break;
    case format::nmea::GgaFixQuality::DeadReckoning:
        metrics.fix_quality = FixQuality::DEAD_RECKONING;
        break;
    }
    system.push(std::move(metrics));
}
