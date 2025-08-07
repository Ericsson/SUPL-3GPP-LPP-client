
#pragma once
#include <core/core.hpp>
#include <ephemeris/ephemeris.hpp>
#include <generator/idokeido/idokeido.hpp>

#include <bitset>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct GNSS_SSR_CodeBias_r15;

namespace idokeido {

class CorrectionCache {
public:
    CorrectionCache()  = default;
    ~CorrectionCache() = default;

    Scalar code_bias(uint8_t iod, SatelliteId satellite_id, SignalId signal_id) NOEXCEPT;

    void add_correction(long gnss_id, GNSS_SSR_CodeBias_r15 const* correction) NOEXCEPT;

private:
    struct IodEntry {
        uint8_t                              mGnss;
        uint8_t                              mIod;
        std::array<Scalar, SATELLITE_ID_MAX * SIGNAL_ID_MAX> mCodeBias;

        Scalar code_bias(SatelliteId satellite_id, SignalId signal_id) NOEXCEPT {
            return mCodeBias[satellite_id.absolute_id() * SIGNAL_ID_MAX + signal_id.absolute_id()];
        }
    };

    IodEntry* get_iod_entry(uint8_t gnss, uint8_t iod) NOEXCEPT;
    IodEntry& create_iod_entry(uint8_t gnss, uint8_t iod) NOEXCEPT;

    std::unordered_map<uint16_t, IodEntry> mData;
};

}  // namespace idokeido
