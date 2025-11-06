#pragma once
#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <format/nav/d1.hpp>
#include <format/nav/gal/inav.hpp>
#include <format/nav/gps/lnav.hpp>
#include <format/ubx/messages/rxm_sfrbx.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"
#include "ubx.hpp"

class Ubx2Eph : public streamline::Inspector<UbxMessage> {
public:
    Ubx2Eph(Ubx2EphConfig const& config) : mConfig(config) {}

    char const* name() const NOEXCEPT override { return "Ubx2Eph"; }
    void        inspect(streamline::System& system, DataType const& message,
                        uint64_t tag) NOEXCEPT override;

private:
    void handle_gps_lnav(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT;
    void handle_gps(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT;
    void handle_gal_inav(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT;
    void handle_gal(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT;
    void handle_bds_d1(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT;
    void handle_bds(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT;

    Ubx2EphConfig const&                   mConfig;
    format::nav::gps::lnav::EphemerisCollector mGpsCollector;
    format::nav::gal::InavEphemerisCollector   mGalCollector;
    format::nav::D1Collector                   mBdsCollector;
};
