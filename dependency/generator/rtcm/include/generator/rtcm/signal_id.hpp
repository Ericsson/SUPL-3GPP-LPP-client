#pragma once
#include <generator/rtcm/maybe.hpp>
#include <generator/rtcm/satellite_id.hpp>

#include <string>

#define SIGNAL_ABS_COUNT 24
#define SIGNAL_ID_MAX (SATELLITE_ID_MAX * SIGNAL_ABS_COUNT)

enum class FrequencyType {
    UNKNOWN,
    L1,
    L2,
    L5,
    L6,
    E1,
    E5a,
    E5b,
    E5,
    E6,
    B1,
    B2,
    B2a,
    B2b,
    B2ab,
    B3,
    G1,
    G2,
    G3,
};

class SignalId {
public:
    SignalId() = default;

    enum Gnss {
        UNKNOWN,
        GPS,
        GLONASS,
        GALILEO,
        BEIDOU,
        QZSS,
    };

    NODISCARD static SignalId from_lpp(Gnss gnss, long id);

    //
    //
    //

    NODISCARD Gnss gnss() const { return mGnss; }
    NODISCARD Maybe<long> as_msm() const;
    NODISCARD long        lpp_id() const;
    NODISCARD std::string   to_string() const;
    NODISCARD const char*   name() const;
    NODISCARD double        frequency() const;
    NODISCARD double        wavelength() const;
    NODISCARD FrequencyType frequency_type() const;

    NODISCARD long absolute_id() const;

    NODISCARD std::string to_rinex() const;

    inline bool operator==(SignalId const& other) const {
        return mGnss == other.mGnss && mLppId == other.mLppId;
    }

    inline bool operator!=(SignalId const& other) const { return !(*this == other); }

    inline bool operator<(SignalId const& other) const {
        if (mGnss < other.mGnss) return true;
        if (mGnss > other.mGnss) return false;
        return mLppId < other.mLppId;
    }

    NODISCARD inline bool is_valid() const { return mGnss != Gnss::UNKNOWN && mLppId != -1; }

private:
    explicit SignalId(Gnss gnss, int32_t lpp_id) : mGnss(gnss), mLppId(lpp_id) {}

    Gnss    mGnss  = Gnss::UNKNOWN;
    int32_t mLppId = -1;

public:
    static SignalId const GPS_L1_CA;
    static SignalId const GPS_L1C;
    static SignalId const GPS_L2C;
    static SignalId const GPS_L5;
    static SignalId const GPS_L1_P;
    static SignalId const GPS_L1_Z_TRACKING;
    static SignalId const GPS_L2_C_A;
    static SignalId const GPS_L2_P;
    static SignalId const GPS_L2_Z_TRACKING;
    static SignalId const GPS_L2_L2C_M;
    static SignalId const GPS_L2_L2C_L;
    static SignalId const GPS_L2_L2C_M_L;
    static SignalId const GPS_L5_I;
    static SignalId const GPS_L5_Q;
    static SignalId const GPS_L5_I_Q;
    static SignalId const GPS_L1_L1C_D;
    static SignalId const GPS_L1_L1C_P;
    static SignalId const GPS_L1_L1C_D_P;

    static SignalId const GLONASS_G1_CA;
    static SignalId const GLONASS_G2_CA;
    static SignalId const GLONASS_G3;
    static SignalId const GLONASS_G1_P;
    static SignalId const GLONASS_G2_P;
    static SignalId const GLONASS_G1A_D;
    static SignalId const GLONASS_G1A_P;
    static SignalId const GLONASS_G1A_D_P;
    static SignalId const GLONASS_G2A_I;
    static SignalId const GLONASS_G2A_P;
    static SignalId const GLONASS_G2A_I_P;
    static SignalId const GLONASS_G3_I;
    static SignalId const GLONASS_G3_Q;
    static SignalId const GLONASS_G3_I_Q;

    static SignalId const GALILEO_E1;
    static SignalId const GALILEO_E5A;
    static SignalId const GALILEO_E5B;
    static SignalId const GALILEO_E6;
    static SignalId const GALILEO_E5A_E5B;
    static SignalId const GALILEO_E1_C_NO_DATA;
    static SignalId const GALILEO_E1_A;
    static SignalId const GALILEO_E1_B_I_NAV_OS_CS_SOL;
    static SignalId const GALILEO_E1_B_C;
    static SignalId const GALILEO_E1_A_B_C;
    static SignalId const GALILEO_E6_C;
    static SignalId const GALILEO_E6_A;
    static SignalId const GALILEO_E6_B;
    static SignalId const GALILEO_E6_B_C;
    static SignalId const GALILEO_E6_A_B_C;
    static SignalId const GALILEO_E5B_I;
    static SignalId const GALILEO_E5B_Q;
    static SignalId const GALILEO_E5B_I_Q;
    static SignalId const GALILEO_E5_A_B_I;
    static SignalId const GALILEO_E5_A_B_Q;
    static SignalId const GALILEO_E5_A_B_I_Q;
    static SignalId const GALILEO_E5A_I;
    static SignalId const GALILEO_E5A_Q;
    static SignalId const GALILEO_E5A_I_Q;

    static SignalId const BEIDOU_B1_I;
    static SignalId const BEIDOU_B1_Q;
    static SignalId const BEIDOU_B1_I_Q;
    static SignalId const BEIDOU_B3_I;
    static SignalId const BEIDOU_B3_Q;
    static SignalId const BEIDOU_B3_I_Q;
    static SignalId const BEIDOU_B2_I;
    static SignalId const BEIDOU_B2_Q;
    static SignalId const BEIDOU_B2_I_Q;

    static SignalId const QZSS_L1_CA;
    static SignalId const QZSS_L1C_D;
    static SignalId const QZSS_L1C_P;
    static SignalId const QZSS_L1C_D_P;
    static SignalId const QZSS_L2C_M;
    static SignalId const QZSS_L2C_L;
    static SignalId const QZSS_L2C_M_L;
    static SignalId const QZSS_L5_I;
    static SignalId const QZSS_L5_Q;
    static SignalId const QZSS_L5_I_Q;
    static SignalId const QZSS_L6_D;
    static SignalId const QZSS_L6_E;
    static SignalId const BEIDOU_B1C_D;
    static SignalId const BEIDOU_B1C_P;
    static SignalId const BEIDOU_B1C_D_P;
    static SignalId const BEIDOU_B2A_D;
    static SignalId const BEIDOU_B2A_P;
    static SignalId const BEIDOU_B2A_D_P;
};

namespace std {
template <>
struct hash<FrequencyType> {
    std::size_t operator()(FrequencyType const& k) const NOEXCEPT {
        return std::hash<int>()(static_cast<int>(k));
    }
};

template <>
struct hash<SignalId> {
    std::size_t operator()(SignalId const& k) const NOEXCEPT {
        auto hash_gnss   = std::hash<int>()(static_cast<int>(k.gnss()));
        auto hash_lpp_id = std::hash<long>()(k.lpp_id());
        return hash_gnss ^ hash_lpp_id;
    }
};
}  // namespace std

class SatelliteSignalId {
public:
    SatelliteSignalId() = default;
    SatelliteSignalId(SatelliteId satellite, SignalId signal)
        : mSatellite(satellite), mSignal(signal) {}

    NODISCARD SatelliteId satellite() const { return mSatellite; }
    NODISCARD SignalId    signal() const { return mSignal; }

    NODISCARD std::string to_string() const;

    inline bool operator==(SatelliteSignalId const& other) const {
        return mSatellite == other.mSatellite && mSignal == other.mSignal;
    }

    inline bool operator!=(SatelliteSignalId const& other) const { return !(*this == other); }

    inline bool operator<(SatelliteSignalId const& other) const {
        if (mSatellite < other.mSatellite) return true;
        if (mSatellite > other.mSatellite) return false;
        return mSignal < other.mSignal;
    }

private:
    SatelliteId mSatellite;
    SignalId    mSignal;
};

namespace std {
template <>
struct hash<SatelliteSignalId> {
    std::size_t operator()(SatelliteSignalId const& k) const NOEXCEPT {
        auto hash_satellite = std::hash<SatelliteId>()(k.satellite());
        auto hash_signal    = std::hash<SignalId>()(k.signal());
        return hash_satellite ^ hash_signal;
    }
};
}  // namespace std
