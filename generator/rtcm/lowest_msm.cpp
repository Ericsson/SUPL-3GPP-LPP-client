#include <cmath>
#include "messages/msm.hpp"
#include "rtk_data.hpp"

namespace generator {
namespace rtcm {

static bool can_be_encoded_as_msm(const Signal& signal, uint32_t msm_version) {
    // RTCM cannot encode DF402 as missing
    if (!signal.lock_time.valid) return false;

    // MSM 4/6 cannot encode DF404
    if (msm_version == 4 || msm_version == 6) {
        if (signal.fine_phase_range_rate.valid) {
            return false;
        }
    }

    auto uses_df400 = msm_version == 4 || msm_version == 5;
    auto uses_df401 = msm_version == 4 || msm_version == 5;
    auto uses_df402 = msm_version == 4 || msm_version == 5;
    auto uses_df403 = msm_version == 4 || msm_version == 5;

#define ROUND(x) static_cast<long>(round(x))

    // fine pseudo range: DF405 vs DF400
    auto df400_encoded = ROUND(signal.fine_pseudo_range.value / RTCM_N2_24);
    auto df400_decoded = df400_encoded * RTCM_N2_24;
    auto df405_encoded = ROUND(signal.fine_pseudo_range.value / RTCM_N2_29);
    auto df405_decoded = df405_encoded * RTCM_N2_29;
    if (fabs(df400_decoded - df405_decoded) > 0.0 && uses_df400) {
        return false;
    }

    // fine phase range: DF406 vs DF401
    auto df401_encoded = ROUND(signal.fine_phase_range.value / RTCM_N2_29);
    auto df401_decoded = df401_encoded * RTCM_N2_29;
    auto df406_encoded = ROUND(signal.fine_phase_range.value / RTCM_N2_31);
    auto df406_decoded = df406_encoded * RTCM_N2_31;
    if (fabs(df401_decoded - df406_decoded) > 0.0 && uses_df401) {
        return false;
    }

    // lock time indiciator: DF407 vs DF402
    auto df402_encoded = to_msm_lock(signal.lock_time.value);
    auto df402_decoded = from_msm_lock(df402_encoded);
    if (df402_decoded != signal.lock_time.value && uses_df402) {
        return false;
    }

    if (signal.carrier_to_noise_ratio.valid) {
        // carrier to noise ratio: DF408 vs DF403
        auto df403_encoded = ROUND(signal.carrier_to_noise_ratio.value);
        auto df403_decoded = df403_encoded;
        auto df408_encoded = ROUND(signal.carrier_to_noise_ratio.value / 0.0625);
        auto df408_decoded = df408_encoded * 0.0625;
        if (fabs(df403_decoded - df408_decoded) > 0.0 && uses_df403) {
            return false;
        }
    }

    return true;
}

uint32_t Signal::lowest_msm_version() const {
    for (uint32_t i = 4; i <= 7; i++) {
        if (can_be_encoded_as_msm(*this, i)) {
            return i;
        }
    }

    return 0;
}

static bool can_be_encoded_as_msm(const Satellite& satellite, uint32_t msm_version) {
    // RTCM cannot encode DF398 as missing
    if (!satellite.rough_range.valid) return false;

    // MSM 4/6 cannot encode "Extended satellite information"
    if (msm_version == 4 || msm_version == 6) {
        if (satellite.id.gnss() == SatelliteId::Gnss::GLONASS) {
            if (satellite.frequency_channel.valid) return false;
        }
    }

    // MSM 4/6 cannot encode DF399
    if (msm_version == 4 || msm_version == 6) {
        if (satellite.rough_phase_range_rate.valid) {
            return false;
        }
    }

    return true;
}

uint32_t Satellite::lowest_msm_version() const {
    for (uint32_t i = 4; i <= 7; i++) {
        if (can_be_encoded_as_msm(*this, i)) {
            return i;
        }
    }

    return 0;
}

uint32_t Observations::lowest_msm_version() const {
    uint32_t msm = 0;
    for (auto& satellite : satellites) {
        auto satellite_msm = satellite.lowest_msm_version();
        if (satellite_msm == 0) return 0;
        if (satellite_msm > msm) {
            msm = satellite_msm;
        }
    }

    for (auto& signal : signals) {
        auto signal_msm = signal.lowest_msm_version();
        if (signal_msm == 0) return 0;
        if (signal_msm > msm) {
            msm = signal_msm;
        }
    }

    return msm;
}

}  // namespace rtcm
}  // namespace generator
