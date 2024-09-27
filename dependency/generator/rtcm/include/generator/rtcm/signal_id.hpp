#pragma once
#include <generator/rtcm/maybe.hpp>

#include <string>

class SignalId {
public:
    SignalId() = default;

    enum Gnss {
        UNKNOWN,
        GPS,
        GLONASS,
        GALILEO,
        BEIDOU,
    };

    NODISCARD static SignalId from_lpp(Gnss gnss, long id);

    //
    //
    //

    NODISCARD Gnss gnss() const { return mGnss; }
    NODISCARD Maybe<long> as_msm() const;
    NODISCARD long        lpp_id() const;
    NODISCARD std::string to_string() const;
    NODISCARD const char* name() const;
    NODISCARD double     frequency() const;
    NODISCARD double     wavelength() const;

    inline bool operator==(SignalId const& other) const {
        return mGnss == other.mGnss && mLppId == other.mLppId;
    }

    inline bool operator!=(SignalId const& other) const { return !(*this == other); }

private:
    explicit SignalId(Gnss gnss, int32_t lpp_id) : mGnss(gnss), mLppId(lpp_id) {}

    Gnss    mGnss  = Gnss::UNKNOWN;
    int32_t mLppId = -1;

public:
    static const SignalId GPS_L1_CA;
    static const SignalId GPS_L1C;
    static const SignalId GPS_L2C;
    static const SignalId GPS_L5;
    static const SignalId GPS_L1_P;
    static const SignalId GPS_L1_Z_TRACKING;
    static const SignalId GPS_L2_C_A;
    static const SignalId GPS_L2_P;
    static const SignalId GPS_L2_Z_TRACKING;
    static const SignalId GPS_L2_L2C_M;
    static const SignalId GPS_L2_L2C_L;
    static const SignalId GPS_L2_L2C_M_L;
    static const SignalId GPS_L5_I;
    static const SignalId GPS_L5_Q;
    static const SignalId GPS_L5_I_Q;
    static const SignalId GPS_L1_L1C_D;
    static const SignalId GPS_L1_L1C_P;
    static const SignalId GPS_L1_L1C_D_P;
};

namespace std {
template <>
struct hash<SignalId> {
    std::size_t operator()(SignalId const& k) const NOEXCEPT {
        auto hash_gnss         = std::hash<int>()(static_cast<int>(k.gnss()));
        auto hash_lpp_id       = std::hash<long>()(k.lpp_id());
        return hash_gnss ^ hash_lpp_id;
    }
};
}  // namespace std


