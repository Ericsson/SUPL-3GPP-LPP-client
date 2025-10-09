#include "nmea.hpp"

#include <algorithm>
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

void NmeaPrint::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();
    for (auto const& print : mConfig.prints) {
        if (!print.nmea_support()) continue;
        if (!print.accept_tag(tag)) continue;
        message->print();
        return;
    }
}

void NmeaOutput::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto sentence = message->sentence();
    auto data     = reinterpret_cast<uint8_t const*>(sentence.data());
    auto size     = sentence.size();
    for (auto const& output : mOutput.outputs) {
        if (!output.nmea_support()) continue;
        if (!output.accept_tag(tag)) {
            XVERBOSEF(OUTPUT_PRINT_MODULE, "tag %llX not accepted", tag);
            continue;
        }
        XDEBUGF(OUTPUT_PRINT_MODULE, "nmea: %s (%zd bytes) tag=%llX", message->prefix().c_str(),
                size, tag);

        ASSERT(output.stage, "stage is null");
        output.stage->write(OUTPUT_FORMAT_NMEA, data, size);
    }
}

NmeaLocation::NmeaLocation(LocationInformationConfig const& config)
    : mGga(nullptr), mVtg(nullptr), mGst(nullptr), mEpe(nullptr), mConfig(config) {}

NmeaLocation::~NmeaLocation() = default;

lpp::VelocityShape NmeaLocation::vtg_to_velocity(VtgMessage const& vtg) const {
    FUNCTION_SCOPE();
    auto speed  = vtg.speed_over_ground();
    auto course = vtg.true_course_over_ground();
    VERBOSEF("speed: %.2f m/s, course: %.2f deg", speed, course);
    return lpp::VelocityShape::horizontal(speed, course);
}

lpp::HorizontalAccuracy NmeaLocation::horizontal_accuracy(double semi_major, double semi_minor,
                                                          double orientation) const {
    FUNCTION_SCOPEF("semi_major: %.2f, semi_minor: %.2f, orientation: %.2f", semi_major, semi_minor,
                    orientation);
    if (mConfig.convert_confidence_95_to_68) {
        VERBOSEF("converting 95%% to 68%% confidence");
        semi_major = semi_major / 2.4477;
        semi_minor = semi_minor / 2.4477;
        VERBOSEF("adjusted: semi_major: %.2f, semi_minor: %.2f", semi_major, semi_minor);
    }

    auto horizontal_accuracy =
        lpp::HorizontalAccuracy::to_ellipse_39(semi_major, semi_minor, orientation);
    if (mConfig.output_ellipse_68) {
        horizontal_accuracy =
            lpp::HorizontalAccuracy::to_ellipse_68(semi_major, semi_minor, orientation);
    }
    if (mConfig.override_horizontal_confidence >= 0.0 &&
        mConfig.override_horizontal_confidence <= 1.0) {
        VERBOSEF("overriding confidence: %.2f", mConfig.override_horizontal_confidence);
        horizontal_accuracy.confidence = mConfig.override_horizontal_confidence;
    }

    TRACEF("horizontal accuracy: confidence=%.2f", horizontal_accuracy.confidence);
    return horizontal_accuracy;
}

lpp::HorizontalAccuracy NmeaLocation::epe_to_horizontal(EpeMessage const& epe) const {
    FUNCTION_SCOPE();
    VERBOSEF("EPE: semi_major=%.2f, semi_minor=%.2f, orientation=%.2f", epe.semi_major(),
             epe.semi_minor(), epe.orientation());
    return horizontal_accuracy(epe.semi_major(), epe.semi_minor(), epe.orientation());
}

lpp::HorizontalAccuracy NmeaLocation::gst_to_horizontal(GstMessage const& gst) const {
    FUNCTION_SCOPE();
    VERBOSEF("GST: semi_major=%.2f, semi_minor=%.2f, orientation=%.2f", gst.semi_major(),
             gst.semi_minor(), gst.orientation());
    return horizontal_accuracy(gst.semi_major(), gst.semi_minor(), gst.orientation());
}

lpp::VerticalAccuracy NmeaLocation::gst_to_vertical(GstMessage const& gst) const {
    FUNCTION_SCOPE();
    auto error = gst.vertical_position_error();
    VERBOSEF("vertical error: %.2f m", error);
    return lpp::VerticalAccuracy::from_1sigma(error);
}

lpp::VerticalAccuracy NmeaLocation::epe_to_vertical(EpeMessage const& epe) const {
    FUNCTION_SCOPE();
    auto error = epe.vertical_position_error();
    VERBOSEF("vertical error: %.2f m", error);
    return lpp::VerticalAccuracy::from_1sigma(error);
}

void NmeaLocation::consume(streamline::System& system, DataType&& message, uint64_t) NOEXCEPT {
    FUNCTION_SCOPEF("%s", message->prefix().c_str());

    auto received_new_message = false;
    auto message_type         = std::string();
    auto gga                  = dynamic_cast<GgaMessage*>(message.get());
    if (gga) {
        TRACEF("received GGA message: %p -> %p", mGga.get(), gga);
        mGga.reset(gga);
        message.release();
        received_new_message = true;
        message_type         = "gga";
    }

    auto vtg = dynamic_cast<VtgMessage*>(message.get());
    if (vtg) {
        TRACEF("received VTG message: %p -> %p", mVtg.get(), vtg);
        mVtg.reset(vtg);
        message.release();
        received_new_message = true;
        message_type         = "vtg";
    }

    auto gst = dynamic_cast<GstMessage*>(message.get());
    if (gst) {
        TRACEF("received GST message: %p -> %p", mGst.get(), gst);
        mGst.reset(gst);
        message.release();
        received_new_message = true;
        message_type         = "gst";
    }

    auto epe = dynamic_cast<EpeMessage*>(message.get());
    if (epe) {
        TRACEF("received EPE message: %p -> %p", mEpe.get(), epe);
        mEpe.reset(epe);
        message.release();
        received_new_message = true;
        message_type         = "epe";
    }

    if (!received_new_message) {
        VERBOSEF("unknown message type (%s), ignoring", message->prefix().c_str());
        return;
    }

    if (!mConfig.nmea_order.empty()) {
        if (mConfig.nmea_order[mOrderReceived.size()] != message_type) {
            if (mConfig.nmea_order_strict) {
                VERBOSEF("unexpected message type (%s, expected %s), resetting messages",
                         message_type.c_str(), mConfig.nmea_order[mOrderReceived.size()].c_str());
                mGga.reset();
                mVtg.reset();
                mGst.reset();
                mEpe.reset();
                mOrderReceived.clear();
                return;
            } else {
                VERBOSEF("ignoring %s (expected %s)", message_type.c_str(),
                         mConfig.nmea_order[mOrderReceived.size()].c_str());
                return;
            }
        } else {
            VERBOSEF("order tracking: received %s (%zu/%zu)", message_type.c_str(),
                     mOrderReceived.size(), mConfig.nmea_order.size());
            mOrderReceived.push_back(message_type);
        }

        if (mOrderReceived.size() != mConfig.nmea_order.size()) {
            VERBOSEF("order tracking: waiting for complete order (%zu/%zu)", mOrderReceived.size(),
                     mConfig.nmea_order.size());
            return;
        }
    }

    if (!mGga) {
        VERBOSEF("nmea location require gga");
        if (!mConfig.nmea_order.empty() && mOrderReceived.size() == mConfig.nmea_order.size()) {
            WARNF("nmea order list completed but no gga message received, gga is required to "
                  "report position");
            VERBOSEF("resetting messages after reporting position");
            mGga.reset();
            mVtg.reset();
            mGst.reset();
            mEpe.reset();
            mOrderReceived.clear();
        }
        return;
    }

    VERBOSEF("GGA fix quality: %d", static_cast<int>(mGga->fix_quality()));

    auto semi_major = 5.0;
    auto semi_minor = 5.0;

    if (mGga->fix_quality() == GgaFixQuality::RtkFixed) {
        VERBOSEF("RTK fixed, using 0.05m accuracy");
        semi_major = 0.05;
        semi_minor = 0.05;
    } else if (mGga->fix_quality() == GgaFixQuality::RtkFloat) {
        VERBOSEF("RTK float, using 0.5m accuracy");
        semi_major = 0.5;
        semi_minor = 0.5;
    } else {
        VERBOSEF("default accuracy: 5.0m");
    }

    auto velocity   = lpp::VelocityShape::horizontal(0.0, 0.0);
    auto horizontal = lpp::HorizontalAccuracy::to_ellipse_39(semi_major, semi_minor, 0.0);
    auto vertical   = lpp::VerticalAccuracy::from_1sigma(1.0);

    if (mGst) {
        VERBOSEF("using GST for accuracy");
        horizontal = gst_to_horizontal(*mGst.get());
        vertical   = gst_to_vertical(*mGst.get());
    } else if (mEpe) {
        VERBOSEF("using EPE for accuracy");
        horizontal = epe_to_horizontal(*mEpe.get());
        vertical   = epe_to_vertical(*mEpe.get());
    } else if (mConfig.nmea_require_gst) {
        VERBOSEF("nmea location require gst/epe");
        return;
    } else {
        VERBOSEF("using default accuracy values");
    }

    if (mVtg) {
        VERBOSEF("using VTG for velocity");
        velocity = vtg_to_velocity(*mVtg.get());
    } else if (mConfig.nmea_require_vtg) {
        VERBOSEF("nmea location require vtg");
        return;
    } else {
        VERBOSEF("using default velocity (0.0 m/s)");
    }

    process(system, *mGga.get(), velocity, horizontal, vertical);

    if (!mConfig.nmea_order.empty()) {
        VERBOSEF("resetting messages after reporting position");
        mGga.reset();
        mVtg.reset();
        mGst.reset();
        mEpe.reset();
        mOrderReceived.clear();
    }
}

void NmeaLocation::process(streamline::System& system, format::nmea::GgaMessage const& gga,
                           lpp::VelocityShape velocity, lpp::HorizontalAccuracy horizontal,
                           lpp::VerticalAccuracy vertical) {
    VSCOPE_FUNCTION();
    VERBOSEF("position: lat=%.8f, lon=%.8f, alt=%.2f", gga.latitude(), gga.longitude(),
             gga.altitude());
    VERBOSEF("satellites: %d, hdop: %.2f", gga.satellites_in_view(), gga.h_dop());

    lpp::LocationInformation location{};
    location.time     = gga.time_of_day();
    location.location = lpp::LocationShape::ha_ellipsoid_altitude_with_uncertainty(
        gga.latitude(), gga.longitude(), gga.altitude(), horizontal, vertical);
    location.velocity = velocity;
    TRACEF("pushing location information");
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
    auto hdop_val = metrics.hdop.value_or(0.0);
    auto age_val  = metrics.age_of_corrections.value_or(0.0);
    TRACEF("pushing GNSS metrics: fix=%d, sats=%d, hdop=%.2f, age=%.2f",
           static_cast<int>(metrics.fix_quality), metrics.number_of_satellites, hdop_val, age_val);
    system.push(std::move(metrics));
}
