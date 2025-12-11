#pragma once
#include <time/tai.hpp>

#include <chrono>
#include <cmath>

namespace lpp {

// TODO: move?
template <typename T>
struct Optional {
    T    value;
    bool valid;

    Optional() : value{}, valid{false} {}
    Optional(T new_value) : value(new_value), valid{true} {}

    Optional(Optional const& other) : value(other.value), valid{other.valid} {}
    Optional(Optional&& other) NOEXCEPT : value(std::move(other.value)), valid{other.valid} {}
    Optional& operator=(Optional const& other) {
        value = other.value;
        valid = other.valid;
        return *this;
    }
    Optional& operator=(Optional&& other) NOEXCEPT {
        value = std::move(other.value);
        valid = other.valid;
        return *this;
    }

    NODISCARD bool     has_value() const { return valid; }
    NODISCARD T const& const_value() const { return value; }

    NODISCARD T value_or(T const& default_value) const {
        if (valid) return value;
        return default_value;
    }

    static Optional<T> invalid() { return Optional<T>{T{}, false}; }
};

struct HorizontalAccuracy {
    // Semi-major axis of the error ellipse in meters.
    double semi_major;
    // Semi-minor axis of the error ellipse in meters.
    double semi_minor;
    // Orientation of the error ellipse in degrees from true north. The orientation is the angle
    // between the semi-major axis and the true north, in the range [0, 180).
    double orientation;
    // Confidence level that the true position is within the error ellipse. It is represented as a
    // percentage in the range [0, 1]. A value of 0 indicates 'no information'. A value of 0.68
    // indicates a 1-sigma confidence level.
    double confidence;

    // Create a HorizontalAccuracy object from 1-sigma semi-major and semi-minor axes in meters
    // and the orientation in degrees from true north. This stores the axes without rescaling and
    // sets the confidence level to 39.3469%, which corresponds to the probability that the true
    // position lies within an error ellipse defined by 1-sigma axes in two dimensions.
    static HorizontalAccuracy from_1sigma(double semi_major, double semi_minor,
                                          double orientation) {
        return HorizontalAccuracy{semi_major, semi_minor, normalize_orientation(orientation),
                                  0.393469};
    }

    // Create a HorizontalAccuracy from UBX h_acc field at 95% confidence, assuming h_acc
    // represents the root-sum-square (RSS) of the semi-major and semi-minor axes. For a circular
    // error ellipse, h_acc = sqrt(semi_95^2 + semi_95^2), so semi_95 = h_acc / sqrt(2). To
    // convert to 1-sigma, we divide by the 95% confidence multiplier (2.4477), giving
    // sigma = h_acc / (sqrt(2) * 2.4477) = h_acc / 3.4616.
    static HorizontalAccuracy from_h_acc_95_rss(double h_acc, double orientation) {
        constexpr double factor = 1.4142135623730951 * 2.4477;  // sqrt(2) * MULT_95 = 3.4616
        auto             sigma  = h_acc / factor;
        return from_1sigma(sigma, sigma, orientation);
    }

    // Create a HorizontalAccuracy from UBX h_acc field at 95% confidence, assuming h_acc
    // represents the radius of a circular error ellipse at 95% confidence. To convert to 1-sigma,
    // we divide by the 95% confidence multiplier (2.4477), giving sigma = h_acc / 2.4477.
    static HorizontalAccuracy from_h_acc_95_radius(double h_acc, double orientation) {
        constexpr double mult_95 = 2.4477;
        auto             sigma   = h_acc / mult_95;
        return from_1sigma(sigma, sigma, orientation);
    }

    // Convert this HorizontalAccuracy to a different confidence level by rescaling the semi-major
    // and semi-minor axes. The orientation remains unchanged. This uses the chi-squared
    // distribution with 2 degrees of freedom to calculate the appropriate scaling factor between
    // the current confidence level and the target confidence level.
    HorizontalAccuracy to_confidence(double target_confidence) const {
        auto mult_current = confidence_to_multiplier(confidence);
        auto mult_target  = confidence_to_multiplier(target_confidence);
        auto scale        = mult_target / mult_current;

        return HorizontalAccuracy{semi_major * scale, semi_minor * scale, orientation,
                                  target_confidence};
    }

    // Convert this HorizontalAccuracy to 39.3469% confidence (1-sigma).
    HorizontalAccuracy to_39() const { return to_confidence(0.393469); }

    // Convert this HorizontalAccuracy to 68.27% confidence.
    HorizontalAccuracy to_68() const { return to_confidence(0.6827); }

    // Convert this HorizontalAccuracy to 95% confidence.
    HorizontalAccuracy to_95() const { return to_confidence(0.95); }

private:
    static double normalize_orientation(double orientation) {
        // Normalize the orientation to the range [0, 180).
        while (orientation < 0)
            orientation += 180;
        while (orientation >= 180)
            orientation -= 180;
        return orientation;
    }

    // Chi-squared inverse CDF for df=2
    // For df=2: chi2(p) = -2 * ln(1 - p)
    static constexpr double chi2_ppf(double p) { return -2.0 * std::log(1.0 - p); }

    // Calculate multiplier from confidence level
    static constexpr double confidence_to_multiplier(double conf) {
        return std::sqrt(chi2_ppf(conf));
    }
};

struct Horizontal {
    // Accuracy of the horizontal position.
    HorizontalAccuracy accuracy;
    // Speed of the moving object in m/s. The speed is in the direction of the bearing.
    double speed;
    // Uncertainty of the speed in m/s for a 1-sigma confidence level.
    double speed_accuracy;
    // Heading in degrees from true north in the range [0, 360).
    double bearing;
};

struct VerticalAccuracy {
    // Uncertainty of the vertical position in meters for the specified confidence level.
    double uncertainty;
    // Confidence level that the true position is within the uncertainty. It is represented as a
    // percentage in the range [0, 1]. A value of 0 indicates 'no information'. A value of 0.68
    // indicates a 1-sigma confidence level.
    double confidence;

    // Create a VerticalAccuracy object from the 1-sigma uncertainty in meters.
    static VerticalAccuracy from_1sigma(double uncertainty) {
        return VerticalAccuracy{uncertainty, 0.68};
    }
};

enum class VerticalDirection {
    Up   = 0,
    Down = 1,
};

struct Vertical {
    // Accuracy of the vertical position.
    VerticalAccuracy accuracy;
    // Speed of the moving object in m/s. The speed is in the direction of the vertical velocity
    // direction.
    double speed;
    // Uncertainty of the speed in m/s for a 1-sigma confidence level.
    double speed_accuracy;
    // Direction of the vertical velocity.
    VerticalDirection direction;
};

struct LocationShape {
    enum class Kind {
        HighAccuracyEllipsoidPointWithUncertaintyEllipse,
        HighAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid,
    };
    Kind kind;
    union {
        struct {
            // Latitude in degrees from the equator in the range [-90, 90].
            double latitude;
            // Longitude in degrees from the prime meridian in the range [-180, 180].
            double longitude;
            // Horizontal accuracy of the location estimate.
            HorizontalAccuracy horizontal_accuracy;
        } haepue;
        struct {
            // Latitude in degrees from the equator in the range [-90, 90].
            double latitude;
            // Longitude in degrees from the prime meridian in the range [-180, 180].
            double longitude;
            // Altitude in meters above the WGS84 ellipsoid.
            double altitude;
            // Horizontal accuracy of the location estimate.
            HorizontalAccuracy horizontal_accuracy;
            // Vertical accuracy of the location estimate.
            VerticalAccuracy vertical_accuracy;
        } haepaue;
    } data;

    NODISCARD double latitude() const {
        switch (kind) {
        case Kind::HighAccuracyEllipsoidPointWithUncertaintyEllipse: return data.haepue.latitude;
        case Kind::HighAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid:
            return data.haepaue.latitude;
        }
        return 0.0;
    }

    NODISCARD double longitude() const {
        switch (kind) {
        case Kind::HighAccuracyEllipsoidPointWithUncertaintyEllipse: return data.haepue.longitude;
        case Kind::HighAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid:
            return data.haepaue.longitude;
        }
        return 0.0;
    }

    NODISCARD double altitude() const {
        switch (kind) {
        case Kind::HighAccuracyEllipsoidPointWithUncertaintyEllipse: return 0.0;
        case Kind::HighAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid:
            return data.haepaue.altitude;
        }
        return 0.0;
    }

    static LocationShape ha_ellipsoid_with_uncertainty(double latitude, double longitude,
                                                       HorizontalAccuracy horizontal_accuracy) {
        LocationShape shape{};
        shape.kind                  = Kind::HighAccuracyEllipsoidPointWithUncertaintyEllipse;
        shape.data.haepue.latitude  = latitude;
        shape.data.haepue.longitude = longitude;
        shape.data.haepue.horizontal_accuracy = horizontal_accuracy;
        return shape;
    }

    static LocationShape
    ha_ellipsoid_altitude_with_uncertainty(double latitude, double longitude, double altitude,
                                           HorizontalAccuracy horizontal_accuracy,
                                           VerticalAccuracy   vertical_accuracy) {
        LocationShape shape{};
        shape.kind = Kind::HighAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid;
        shape.data.haepaue.latitude            = latitude;
        shape.data.haepaue.longitude           = longitude;
        shape.data.haepaue.altitude            = altitude;
        shape.data.haepaue.horizontal_accuracy = horizontal_accuracy;
        shape.data.haepaue.vertical_accuracy   = vertical_accuracy;
        return shape;
    }
};

struct VelocityShape {
    enum class Kind {
        HorizontalVelocity,
        HorizontalWithVerticalVelocity,
        HorizontalVelocityWithUncertainty,
        HorizontalWithVerticalVelocityAndUncertainty,
    };

    Kind kind;
    union {
        struct {
            // Horizontal velocity
            struct {
                // Speed of the moving object in m/s.
                double speed;
                // Heading in degrees from true north in the range [0, 360).
                double bearing;
            } horizontal;
        } hv;
        struct {
            // Horizontal velocity
            struct {
                // Speed of the moving object in m/s.
                double speed;
                // Heading in degrees from true north in the range [0, 360).
                double bearing;
            } horizontal;
            // Vertical velocity
            struct {
                // Speed of the moving object in m/s.
                double speed;
                // Direction of the vertical velocity.
                VerticalDirection direction;
            } vertical;
        } hvv;
        struct {
            // Horizontal velocity with uncertainty.
            struct {
                // Speed of the moving object in m/s.
                double speed;
                // Uncertainty of the speed in m/s for a 1-sigma confidence level.
                double uncertainty;
                // Heading in degrees from true north in the range [0, 360).
                double bearing;
            } horizontal;
        } hvu;
        struct {
            // Horizontal velocity with uncertainty.
            struct {
                // Speed of the moving object in m/s.
                double speed;
                // Uncertainty of the speed in m/s for a 1-sigma confidence level.
                double uncertainty;
                // Heading in degrees from true north in the range [0, 360).
                double bearing;
            } horizontal;
            // Vertical velocity with uncertainty.
            struct {
                // Speed of the moving object in m/s.
                double speed;
                // Uncertainty of the speed in m/s for a 1-sigma confidence level.
                double uncertainty;
                // Direction of the vertical velocity.
                VerticalDirection direction;
            } vertical;
        } hvvu;
    } data;

    static VelocityShape horizontal(double horizontal_speed, double bearing) {
        VelocityShape shape{};
        shape.kind                       = Kind::HorizontalVelocity;
        shape.data.hv.horizontal.speed   = horizontal_speed;
        shape.data.hv.horizontal.bearing = bearing;
        return shape;
    }

    static VelocityShape horizontal_vertical(double horizontal_speed, double bearing,
                                             double vertical_speed, VerticalDirection direction) {
        VelocityShape shape{};
        shape.kind                        = Kind::HorizontalWithVerticalVelocity;
        shape.data.hvv.horizontal.speed   = horizontal_speed;
        shape.data.hvv.horizontal.bearing = bearing;
        shape.data.hvv.vertical.speed     = vertical_speed;
        shape.data.hvv.vertical.direction = direction;
        return shape;
    }

    static VelocityShape horizontal_with_uncertainty(double horizontal_speed,
                                                     double horizontal_uncertainty,
                                                     double bearing) {
        VelocityShape shape{};
        shape.kind                            = Kind::HorizontalVelocityWithUncertainty;
        shape.data.hvu.horizontal.speed       = horizontal_speed;
        shape.data.hvu.horizontal.uncertainty = horizontal_uncertainty;
        shape.data.hvu.horizontal.bearing     = bearing;
        return shape;
    }

    static VelocityShape horizontal_vertical_with_uncertainty(double horizontal_speed,
                                                              double horizontal_uncertainty,
                                                              double bearing, double vertical_speed,
                                                              double vertical_uncertainty,
                                                              VerticalDirection direction) {
        VelocityShape shape{};
        shape.kind                             = Kind::HorizontalWithVerticalVelocityAndUncertainty;
        shape.data.hvvu.horizontal.speed       = horizontal_speed;
        shape.data.hvvu.horizontal.uncertainty = horizontal_uncertainty;
        shape.data.hvvu.horizontal.bearing     = bearing;
        shape.data.hvvu.vertical.speed         = vertical_speed;
        shape.data.hvvu.vertical.uncertainty   = vertical_uncertainty;
        shape.data.hvvu.vertical.direction     = direction;
        return shape;
    }
};

struct LocationInformation {
    // Time of the location estimate.
    ts::Tai time;

    // Shape of the location estimate.
    Optional<LocationShape> location;
    // Shape of the velocity estimate.
    Optional<VelocityShape> velocity;
};

enum class FixQuality : unsigned char {
    // invalid, or no position available, E.g u-blox F9P UBX-NAV-PVT attribute fixType = 0.
    INVALID = 0,
    // Standalone GNSS without correction data, E.g u-blox F9P UBX-NAV-PVT attribute fixType = 2 or
    // 3 (and carrSoln = 0)
    STANDALONE = 1,
    // DGPS fix
    DgpsFix = 2,
    // PPS fix
    PpsFix = 3,
    // RTK fix. E.g u-blox F9P UBX-NAV-PVT attribute fixType = 3 and carrSoln = 2
    RtkFix = 4,
    // RTK float. E.g u-blox F9P UBX-NAV-PVT attribute fixType = 3 and carrSoln = 1
    RtkFloat = 5,
    // Estimated fix (dead reckoning). E.g u-blox F9P UBX-NAV-PVT attribute fixType = 1
    DeadReckoning = 6,
    // Manual input mode
    ManualInput = 7,
    // Simulation mode
    SIMULATION = 8,
    // WAAS fix (not NMEA standard, but NovAtel receivers report this instead of a 2)
    WaasFix = 9,
};

struct HaGnssMetrics {
    // RTK fix quality.
    FixQuality fix_quality;
    // Number of satellites used in the navigation solution [0, 64].
    long number_of_satellites;
    // Age of the most recently used assistance data in seconds [0, 9.9].
    Optional<double> age_of_corrections;
    // Horizontal dilution of precision in range [0.1, 25.6].
    Optional<double> hdop;
    // 3D dilution of precision in range [0.1, 25.6].
    Optional<double> pdop;
};

struct CoordinateType {
    bool ellipsoid_point;
    bool ellipsoid_point_with_uncertainty_circle;
    bool ellipsoid_point_with_uncertainty_ellipse;
    bool polygon;
    bool ellipsoid_point_with_altitude;
    bool ellipsoid_point_with_altitude_and_uncertainty_ellipsoid;
    bool ellipsoid_arc;
    bool ha_ellipsoid_point_with_uncertainty_ellipse;
    bool ha_ellipsoid_point_with_altitude_and_uncertainty_ellipsoid;
    bool ha_ellipsoid_point_with_scalable_uncertainty_ellipse;
    bool ha_ellipsoid_point_with_scalable_altitude_and_uncertainty_ellipse;
    bool local2d_point_with_uncertainty_ellipse;
    bool local3d_point_with_uncertainty_elipsoid;
};

struct VelocityType {
    bool horizontal;
    bool horizontal_with_vertical;
    bool horizontal_with_uncertainty;
    bool horizontal_with_vertical_and_uncertainty;
};

struct PeriodicLocationInformationDeliveryDescription {
    bool                                ha_gnss_metrics;
    bool                                reporting_amount_unlimited;
    long                                reporting_amount;
    std::chrono::steady_clock::duration reporting_interval;

    CoordinateType coordinate_type;
    VelocityType   velocity_type;
};

}  // namespace lpp

namespace streamline {
template <typename T>
struct TypeName;

template <>
struct TypeName<lpp::LocationInformation> {
    static char const* name() { return "lpp::LocationInformation"; }
};

template <>
struct TypeName<lpp::HaGnssMetrics> {
    static char const* name() { return "lpp::HaGnssMetrics"; }
};
}  // namespace streamline
