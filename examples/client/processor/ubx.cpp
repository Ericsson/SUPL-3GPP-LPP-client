#include "ubx.hpp"

#include <format/ubx/messages/nav_pvt.hpp>
#include <loglet/loglet.hpp>
#include <lpp/location_information.hpp>

#include <cmath>

LOGLET_MODULE2(p, ubx);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, ubx)

void UbxPrint::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();
    for (auto const& print : mConfig.prints) {
        if (!print.ubx_support()) continue;
        if (!print.accept_tag(tag)) continue;
        message->print();
        return;
    }
}

void UbxOutput::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto& data = message->data();
    for (auto& output : mOutput.outputs) {
        if (!output.ubx_support()) continue;
        if (!output.accept_tag(tag)) {
            XDEBUGF(OUTPUT_PRINT_MODULE, "tag %llX not accepted", tag);
            continue;
        }
        if (output.print) {
            XINFOF(OUTPUT_PRINT_MODULE, "ubx: %zd bytes", data.size());
        }

        ASSERT(output.stage, "stage is null");
        output.stage->write(OUTPUT_FORMAT_UBX, data.data(), data.size());
    }
}

void UbxLocation::inspect(streamline::System& system, DataType const& message, uint64_t) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto nav_pvt = dynamic_cast<format::ubx::UbxNavPvt*>(message.get());
    if (nav_pvt) {
        this->nav_pvt(system, *nav_pvt);
    }
}

void UbxLocation::nav_pvt(streamline::System& system, format::ubx::UbxNavPvt const& nav_pvt) {
    VSCOPE_FUNCTION();

    if (!nav_pvt.valid_time()) return;

    // NOTE(ewasjon): We need to divide the horizontal accuracy by sqrt(2) to get the semi-major and
    // semi-minor axes. We assume that the semi-major and semi-minor axes are equal.
    // h_acc = sqrt(semi_major^2 + semi_minor^2)
    // h_acc = sqrt(2 * semi_major^2) => semi_major = h_acc / sqrt(2)
    auto semi_major = nav_pvt.h_acc() / 1.4142135623730951;
    auto semi_minor = nav_pvt.h_acc() / 1.4142135623730951;

    // TODO(ewasjon): Is this really relavant for UBX-NAV-PVT?
    if (mConfig.convert_confidence_95_to_68) {
        // 95% confidence to 68% confidence
        // TODO(ewasjon): should this not be 1.95996 (sqrt(3.84)) from 1-degree chi-squared
        // distribution?
        semi_major = semi_major / 2.4477;
        semi_minor = semi_minor / 2.4477;
    }

    auto horizontal_accuracy = lpp::HorizontalAccuracy::to_ellipse_39(semi_major, semi_minor, 0);
    if (mConfig.output_ellipse_68) {
        horizontal_accuracy = lpp::HorizontalAccuracy::to_ellipse_68(semi_major, semi_minor, 0);
    }
    if (mConfig.override_horizontal_confidence >= 0.0 &&
        mConfig.override_horizontal_confidence <= 1.0) {
        horizontal_accuracy.confidence = mConfig.override_horizontal_confidence;
    }

    auto location_shape = lpp::LocationShape::ha_ellipsoid_altitude_with_uncertainty(
        nav_pvt.latitude(), nav_pvt.longitude(), nav_pvt.altitude(), horizontal_accuracy,
        lpp::VerticalAccuracy::from_1sigma(nav_pvt.v_acc()));

    auto velocity_shape = lpp::VelocityShape::horizontal_vertical_with_uncertainty(
        nav_pvt.h_vel(), nav_pvt.h_vel_acc(), nav_pvt.head_mot(), fabs(nav_pvt.v_vel()),
        nav_pvt.v_vel_acc(),
        nav_pvt.v_vel() > 0 ? lpp::VerticalDirection::Down : lpp::VerticalDirection::Up);

    lpp::LocationInformation location{};
    location.time     = nav_pvt.tai_time();
    location.location = location_shape;
    location.velocity = velocity_shape;
    system.push(std::move(location));

    lpp::HaGnssMetrics metrics{};
    metrics.fix_quality = lpp::FixQuality::INVALID;
    if (nav_pvt.fix_type() == 3) {
        if (nav_pvt.carr_soln() == 2) {
            metrics.fix_quality = lpp::FixQuality::RTK_FIX;
        } else if (nav_pvt.carr_soln() == 1) {
            metrics.fix_quality = lpp::FixQuality::RTK_FLOAT;
        } else {
            metrics.fix_quality = lpp::FixQuality::STANDALONE;
        }
    } else if (nav_pvt.fix_type() == 2) {
        metrics.fix_quality = lpp::FixQuality::STANDALONE;
    } else if (nav_pvt.fix_type() == 1) {
        metrics.fix_quality = lpp::FixQuality::DEAD_RECKONING;
    }

    auto age_of_correction_data = nav_pvt.age_of_correction_data();
    if (age_of_correction_data > 0) {
        if (age_of_correction_data >= 10.0) age_of_correction_data = 9.9;
        if (age_of_correction_data < 0.0) age_of_correction_data = 0.0;
        metrics.age_of_corrections = age_of_correction_data;
    }

    metrics.number_of_satellites = nav_pvt.num_sv();
    metrics.pdop                 = nav_pvt.p_dop();
    system.push(std::move(metrics));
}
