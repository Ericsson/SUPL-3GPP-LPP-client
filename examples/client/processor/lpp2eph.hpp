#pragma once
#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <lpp/message.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"

struct GNSS_NavigationModel;

class Lpp2Eph : public streamline::Inspector<lpp::Message> {
public:
    Lpp2Eph(Lpp2EphConfig const& config) : mConfig(config) {}

    NODISCARD char const* name() const NOEXCEPT override { return "Lpp2Eph"; }
    void                  inspect(streamline::System& system, DataType const& message,
                                  uint64_t tag) NOEXCEPT override;

private:
    void process_gps_navigation_model(streamline::System&         system,
                                      GNSS_NavigationModel const& nav_model) NOEXCEPT;
    void process_gal_navigation_model(streamline::System&         system,
                                      GNSS_NavigationModel const& nav_model) NOEXCEPT;
    void process_bds_navigation_model(streamline::System&         system,
                                      GNSS_NavigationModel const& nav_model) NOEXCEPT;

    Lpp2EphConfig const& mConfig;
};
