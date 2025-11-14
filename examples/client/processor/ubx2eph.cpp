#include "ubx2eph.hpp"
#include <loglet/loglet.hpp>

LOGLET_MODULE2(p, ubx2eph);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, ubx2eph)

void Ubx2Eph::handle_gps_lnav(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto words = format::nav::Words::from_sfrbx_l1ca(sfrbx->words());

    format::nav::gps::lnav::Subframe subframe{};
    if (!format::nav::gps::lnav::Subframe::decode(words, subframe)) {
        WARNF("failed to decode GPS LNAV subframe");
        return;
    }

    ephemeris::GpsEphemeris ephemeris{};
    if (!mGpsCollector.process(sfrbx->sv_id(), subframe, ephemeris)) return;

    system.push(std::move(ephemeris));
}

void Ubx2Eph::handle_gps(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mConfig.gps) return;
    if (sfrbx->sig_id() == 0) {
        handle_gps_lnav(system, sfrbx);
    } else {
        VERBOSEF("unsupported GPS signal id %d", sfrbx->sig_id());
    }
}

void Ubx2Eph::handle_gal_inav(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto words = format::nav::Words::from_sfrbx_e5b(sfrbx->words());

    format::nav::gal::InavWord word{};
    if (!format::nav::gal::InavWord::decode(words, word)) {
        WARNF("failed to decode GAL INAV word");
        return;
    }

    ephemeris::GalEphemeris ephemeris{};
    if (!mGalCollector.process(sfrbx->sv_id(), word, ephemeris)) return;

    system.push(std::move(ephemeris));
}

void Ubx2Eph::handle_gal(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mConfig.galileo) return;
    if (sfrbx->sig_id() == 5) {
        handle_gal_inav(system, sfrbx);
    } else {
        VERBOSEF("unsupported GAL signal id %d", sfrbx->sig_id());
    }
}

void Ubx2Eph::handle_bds_d1(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto words = format::nav::Words::from_sfrbx_bds_d1(sfrbx->words());

    format::nav::D1Subframe subframe{};
    if (!format::nav::D1Subframe::decode(words, sfrbx->sv_id(), subframe)) {
        WARNF("failed to decode BDS D1 subframe");
        return;
    }

    ephemeris::BdsEphemeris ephemeris{};
    if (!mBdsCollector.process(sfrbx->sv_id(), subframe, ephemeris)) return;

    system.push(std::move(ephemeris));
}

void Ubx2Eph::handle_bds(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mConfig.beidou) return;
    if (sfrbx->sig_id() == 0) {
        handle_bds_d1(system, sfrbx);
    } else {
        VERBOSEF("unsupported BDS signal id %d", sfrbx->sig_id());
    }
}

void Ubx2Eph::inspect(streamline::System& system, DataType const& message, uint64_t) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto ptr = message.get();
    if (!ptr) return;

    auto sfrbx = dynamic_cast<format::ubx::RxmSfrbx*>(ptr);
    if (!sfrbx) return;

    if (sfrbx->gnss_id() == 0) {
        handle_gps(system, sfrbx);
    } else if (sfrbx->gnss_id() == 2) {
        handle_gal(system, sfrbx);
    } else if (sfrbx->gnss_id() == 3) {
        handle_bds(system, sfrbx);
    }
}
