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

    system.push(std::move(ephemeris), mTag);
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

    system.push(std::move(ephemeris), mTag);
}

void Ubx2Eph::handle_gal(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mConfig.galileo) return;
    // sig_id 1 = E1-B I/NAV (ZED-X20P), sig_id 5 = E1-B I/NAV (older receivers)
    if (sfrbx->sig_id() == 1 || sfrbx->sig_id() == 5) {
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

    system.push(std::move(ephemeris), mTag);
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

void Ubx2Eph::handle_qzs(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mConfig.qzss) return;
    if (sfrbx->sig_id() != 0) {
        VERBOSEF("unsupported QZSS signal id %d", sfrbx->sig_id());
        return;
    }

    // QZSS L1 C/A LNAV has the GPS LNAV layout
    auto                             words = format::nav::Words::from_sfrbx_l1ca(sfrbx->words());
    format::nav::gps::lnav::Subframe subframe{};
    if (!format::nav::gps::lnav::Subframe::decode(words, subframe)) {
        WARNF("failed to decode QZSS LNAV subframe");
        return;
    }

    auto                    prn = static_cast<uint8_t>(192 + sfrbx->sv_id());
    ephemeris::GpsEphemeris gps{};
    if (!mQzsCollector.process(prn, subframe, gps)) return;
    gps.prn = prn;
    system.push(ephemeris::QzsEphemeris::from_gps_layout(gps), mTag);
}

void Ubx2Eph::handle_glo(streamline::System& system, format::ubx::RxmSfrbx* sfrbx) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mConfig.glonass) return;
    if (sfrbx->words().size() < 4) return;
    // the slot is unknown (255) until the receiver has decoded it
    if (sfrbx->sv_id() < 1 || sfrbx->sv_id() > 24) return;

    auto string  = format::nav::GloString::from_sfrbx(sfrbx->words().data());
    auto channel = static_cast<int8_t>(static_cast<int>(sfrbx->freq_id()) - 7);

    ephemeris::GloEphemeris ephemeris{};
    if (!mGloCollector.process(sfrbx->sv_id(), channel, string, ts::Tai::now(), ephemeris)) return;
    system.push(std::move(ephemeris), mTag);
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
    } else if (sfrbx->gnss_id() == 5) {
        handle_qzs(system, sfrbx);
    } else if (sfrbx->gnss_id() == 6) {
        handle_glo(system, sfrbx);
    }
}
