#pragma once
#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/glo.hpp>
#include <ephemeris/gps.hpp>
#include <ephemeris/qzs.hpp>
#include <lpp/message.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>
#include <time/tai.hpp>

#include <unordered_map>

#include "config.hpp"

struct GNSS_NavigationModel;
struct GNSS_AuxiliaryInformation;

class Lpp2Eph : public streamline::Inspector<lpp::Message> {
public:
    Lpp2Eph(Lpp2EphConfig const& config, uint64_t tag) : mConfig(config), mTag(tag) {}

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
    void process_qzs_navigation_model(streamline::System&         system,
                                      GNSS_NavigationModel const& nav_model) NOEXCEPT;
    void process_glo_navigation_model(streamline::System&         system,
                                      GNSS_NavigationModel const& nav_model) NOEXCEPT;
    void process_glo_auxiliary_information(GNSS_AuxiliaryInformation const& aux) NOEXCEPT;

    Lpp2EphConfig const& mConfig;
    uint64_t             mTag;
    // GLONASS frequency channel by slot, from GNSS-AuxiliaryInformation
    std::unordered_map<long, int8_t> mGloChannels;
    ts::Tai                          mReferenceTime;
    bool                             mHasReferenceTime{false};
};
