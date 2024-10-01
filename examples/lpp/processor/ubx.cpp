#include "ubx.hpp"

#include <cmath>

#include <format/ubx/messages/nav_pvt.hpp>
#include <lpp/location_information.h>

void UbxPrint::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    message->print();
}

void UbxOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    auto& data = message->data();
    for (auto& output : mOptions.outputs) {
        if ((output.format & OUTPUT_FORMAT_UBX) != 0) {
            output.interface->write(data.data(), data.size());
        }
    }
}

void UbxLocation::inspect(streamline::System& system, DataType const& message) NOEXCEPT {
    auto nav_pvt = dynamic_cast<format::ubx::UbxNavPvt*>(message.get());
    if (nav_pvt) {
        this->nav_pvt(system, *nav_pvt);
    }
}

void UbxLocation::nav_pvt(streamline::System& system, format::ubx::UbxNavPvt const& nav_pvt) {
    using namespace location_information;

    if (!nav_pvt.valid_time()) return;

    // NOTE(ewasjon): We need to divide the horizontal accuracy by sqrt(2) to get the semi-major and
    // semi-minor axes. We assume that the semi-major and semi-minor axes are equal.
    // h_acc = sqrt(semi_major^2 + semi_minor^2)
    // h_acc = sqrt(2 * semi_major^2) => semi_major = h_acc / sqrt(2)
    auto semi_major = nav_pvt.h_acc() / 1.4142135623730951;
    auto semi_minor = nav_pvt.h_acc() / 1.4142135623730951;

    // TODO(ewasjon): Is this really relavant for UBX-NAV-PVT?
    if (mConvertConfidence95To68) {
        // 95% confidence to 68% confidence
        // TODO(ewasjon): should this not be 1.95996 (sqrt(3.84)) from 1-degree chi-squared
        // distribution?
        semi_major = semi_major / 2.4477;
        semi_minor = semi_minor / 2.4477;
    }

    auto horizontal_accuracy = HorizontalAccuracy::to_ellipse_39(semi_major, semi_minor, 0);
    if (mOutputEllipse68) {
        horizontal_accuracy = HorizontalAccuracy::to_ellipse_68(semi_major, semi_minor, 0);
    }
    if (mOverrideHorizontalConfidence >= 0.0) {
        horizontal_accuracy.confidence = mOverrideHorizontalConfidence;
    }

    auto location_shape = LocationShape::ha_ellipsoid_altitude_with_uncertainty(
        nav_pvt.latitude(), nav_pvt.longitude(), nav_pvt.altitude(), horizontal_accuracy,
        VerticalAccuracy::from_1sigma(nav_pvt.v_acc()));

    auto velocity_shape = VelocityShape::horizontal_vertical_with_uncertainty(
        nav_pvt.h_vel(), nav_pvt.h_vel_acc(), nav_pvt.head_mot(), fabs(nav_pvt.v_vel()),
        nav_pvt.v_vel_acc(), nav_pvt.v_vel() > 0 ? VerticalDirection::Down : VerticalDirection::Up);

    LocationInformation location{};
    location.time     = nav_pvt.tai_time();
    location.location = location_shape;
    location.velocity = velocity_shape;
    system.push(std::move(location));

    HaGnssMetrics metrics{};
    metrics.fix_quality = FixQuality::INVALID;
    if (nav_pvt.fix_type() == 3) {
        if (nav_pvt.carr_soln() == 2) {
            metrics.fix_quality = FixQuality::RTK_FIX;
        } else if (nav_pvt.carr_soln() == 1) {
            metrics.fix_quality = FixQuality::RTK_FLOAT;
        } else {
            metrics.fix_quality = FixQuality::STANDALONE;
        }
    } else if (nav_pvt.fix_type() == 2) {
        metrics.fix_quality = FixQuality::STANDALONE;
    } else if (nav_pvt.fix_type() == 1) {
        metrics.fix_quality = FixQuality::DEAD_RECKONING;
    }

    metrics.number_of_satellites = nav_pvt.num_sv();
    metrics.pdop                 = nav_pvt.p_dop();
    system.push(std::move(metrics));
}