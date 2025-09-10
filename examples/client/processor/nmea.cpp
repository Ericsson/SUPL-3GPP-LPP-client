#include "nmea.hpp"

#include <cmath>
#include <format/nmea/epe.hpp>
#include <format/nmea/gga.hpp>
#include <format/nmea/gst.hpp>
#include <format/nmea/vtg.hpp>
#include <loglet/loglet.hpp>
#include <lpp/location_information.hpp>

LOGLET_MODULE2(p, nmea);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, nmea)

using namespace format::nmea;

void NmeaPrint::inspect(streamline::System&, DataType const& message, uint64_t) NOEXCEPT {
    VSCOPE_FUNCTION();
    message->print();
}

void NmeaOutput::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto sentence = message->sentence();
    auto data     = reinterpret_cast<uint8_t const*>(sentence.data());
    auto size     = sentence.size();
    for (auto const& output : mOutput.outputs) {
        if (!output.nmea_support()) continue;
        if (!output.accept_tag(tag)) {
            XDEBUGF(OUTPUT_PRINT_MODULE, "tag %llX not accepted", tag);
            continue;
        }
        if (output.print) {
            XINFOF(OUTPUT_PRINT_MODULE, "nmea: %zd bytes", size);
        }

        ASSERT(output.stage, "stage is null");
        output.stage->write(OUTPUT_FORMAT_NMEA, data, size);
    }
}

NmeaLocation::NmeaLocation(LocationInformationConfig const& config)
    : mGga(nullptr), mVtg(nullptr), mGst(nullptr), mEpe(nullptr), mConfig(config) {}

NmeaLocation::~NmeaLocation() = default;

lpp::VelocityShape NmeaLocation::vtg_to_velocity(VtgMessage const& vtg) const {
    return lpp::VelocityShape::horizontal(vtg.speed_over_ground(), vtg.true_course_over_ground());
}

lpp::HorizontalAccuracy NmeaLocation::horizontal_accuracy(double semi_major, double semi_minor,
                                                          double orientation) const {
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

    return horizontal_accuracy;
}

lpp::HorizontalAccuracy NmeaLocation::epe_to_horizontal(EpeMessage const& epe) const {
    return horizontal_accuracy(epe.semi_major(), epe.semi_minor(), epe.orientation());
}

lpp::HorizontalAccuracy NmeaLocation::gst_to_horizontal(GstMessage const& gst) const {
    return horizontal_accuracy(gst.semi_major(), gst.semi_minor(), gst.orientation());
}

lpp::VerticalAccuracy NmeaLocation::gst_to_vertical(GstMessage const& gst) const {
    return lpp::VerticalAccuracy::from_1sigma(gst.vertical_position_error());
}

lpp::VerticalAccuracy NmeaLocation::epe_to_vertical(EpeMessage const& epe) const {
    return lpp::VerticalAccuracy::from_1sigma(epe.vertical_position_error());
}

void NmeaLocation::consume(streamline::System& system, DataType&& message, uint64_t) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto received_new_message = false;
    auto gga                  = dynamic_cast<GgaMessage*>(message.get());
    if (gga) {
        mGga.reset(gga);
        message.release();
        received_new_message = true;
    }

    auto vtg = dynamic_cast<VtgMessage*>(message.get());
    if (vtg) {
        mVtg.reset(vtg);
        message.release();
        received_new_message = true;
    }

    auto gst = dynamic_cast<GstMessage*>(message.get());
    if (gst) {
        mGst.reset(gst);
        message.release();
        received_new_message = true;
    }

    auto epe = dynamic_cast<EpeMessage*>(message.get());
    if (epe) {
        mEpe.reset(epe);
        message.release();
        received_new_message = true;
    }

    if (!received_new_message) {
        return;
    }

    if (!mGga) {
        DEBUGF("nmea location require gga");
        return;
    }

    auto semi_major = 5.0;
    auto semi_minor = 5.0;

    if (mGga->fix_quality() == GgaFixQuality::RtkFixed) {
        semi_major = 0.05;
        semi_minor = 0.05;
    } else if (mGga->fix_quality() == GgaFixQuality::RtkFloat) {
        semi_major = 0.5;
        semi_minor = 0.5;
    }

    auto velocity   = lpp::VelocityShape::horizontal(0.0, 0.0);
    auto horizontal = lpp::HorizontalAccuracy::to_ellipse_39(semi_major, semi_minor, 0.0);
    auto vertical   = lpp::VerticalAccuracy::from_1sigma(1.0);

    if (mGst) {
        horizontal = gst_to_horizontal(*mGst.get());
        vertical   = gst_to_vertical(*mGst.get());
    } else if (mEpe) {
        horizontal = epe_to_horizontal(*mEpe.get());
        vertical   = epe_to_vertical(*mEpe.get());
    } else if (mConfig.nmea_require_gst) {
        DEBUGF("nmea location require gst/epe");
        return;
    }

    if (mVtg) {
        velocity = vtg_to_velocity(*mVtg.get());
    } else if (mConfig.nmea_require_vtg) {
        DEBUGF("nmea location require vtg");
        return;
    }

    process(system, *mGga.get(), velocity, horizontal, vertical);
}

void NmeaLocation::process(streamline::System& system, format::nmea::GgaMessage const& gga,
                           lpp::VelocityShape velocity, lpp::HorizontalAccuracy horizontal,
                           lpp::VerticalAccuracy vertical) {
    VSCOPE_FUNCTION();
    lpp::LocationInformation location{};
    location.time     = gga.time_of_day();
    location.location = lpp::LocationShape::ha_ellipsoid_altitude_with_uncertainty(
        gga.latitude(), gga.longitude(), gga.altitude(), horizontal, vertical);
    location.velocity = velocity;
    system.push(std ::move(location));

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
