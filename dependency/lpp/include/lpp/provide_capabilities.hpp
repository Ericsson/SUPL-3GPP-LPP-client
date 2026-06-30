#pragma once
#include <lpp/message.hpp>

namespace lpp {

struct GnssCapability {
    bool standalone;   // include standalone in agnss-Modes
    bool ue_based;     // include ue-based in agnss-Modes
    bool ue_assisted;  // include ue-assisted in agnss-Modes
    bool ha_modes;     // include ha-gnss-Modes-r15
};

struct ProvideCapabilities {
    /// @brief Assistance data support
    struct {
        bool osr;
        bool ssr;
        bool unsolicited_periodic;
    } assistance_data;

    /// @brief The GNSS systems that are supported
    struct {
        bool           gps;
        bool           glonass;
        bool           galileo;
        bool           beidou;
        GnssCapability gps_cap;
        GnssCapability glonass_cap;
        GnssCapability galileo_cap;
        GnssCapability beidou_cap;
    } gnss;

    /// @brief Common capability options
    struct {
        bool velocity;              // velocityMeasurementSupport + velocityTypes
        bool reference_location;    // gnss-ReferenceLocationSupport in common
        bool location_coord_types;  // locationCoordinateTypes
        bool no_ecid;               // omit ecid-ProvideCapabilities
    } common;
};

/// Default GnssCapability matching the current "working" behavior
inline GnssCapability default_gnss_capability() {
    return {false, true, true, true};
}

Message create_provide_capabilities(ProvideCapabilities const& capabilities);

}  // namespace lpp
