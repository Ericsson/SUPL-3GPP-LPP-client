#pragma once
#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <format/rtcm/1019.hpp>
#include <format/rtcm/1042.hpp>
#include <format/rtcm/1046.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"
#include "rtcm.hpp"

class Rtcm2Eph : public streamline::Inspector<RtcmMessage> {
public:
    Rtcm2Eph(Rtcm2EphConfig const& config) : mConfig(config) {}

    char const* name() const NOEXCEPT override { return "Rtcm2Eph"; }
    void        inspect(streamline::System& system, DataType const& message,
                        uint64_t tag) NOEXCEPT override;

private:
    void handle_gps_lnav(streamline::System& system, format::rtcm::Rtcm1019* rtcm) NOEXCEPT;
    void handle_gps(streamline::System& system, format::rtcm::Rtcm1019* rtcm) NOEXCEPT;
    void handle_gal_inav(streamline::System& system, format::rtcm::Rtcm1046* rtcm) NOEXCEPT;
    void handle_gal(streamline::System& system, format::rtcm::Rtcm1046* rtcm) NOEXCEPT;
    void handle_bds_d1(streamline::System& system, format::rtcm::Rtcm1042* rtcm) NOEXCEPT;
    void handle_bds(streamline::System& system, format::rtcm::Rtcm1042* rtcm) NOEXCEPT;

    Rtcm2EphConfig const& mConfig;
};
