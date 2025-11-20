#pragma once
#include <core/core.hpp>
#include <ephemeris/result.hpp>
#include <maths/float3.hpp>
#include <msgpack/msgpack.hpp>
#include <time/glo.hpp>

#include <cmath>

namespace ephemeris {

struct GloEphemeris {
    uint8_t slot_number;
    int8_t  frequency_number;

    ts::Glo reference_time;

    Float3 position;
    Float3 velocity;
    Float3 acceleration;

    double tau_n;
    double gamma_n;

    uint8_t health;
    uint8_t age;
    uint8_t m;

    uint16_t lpp_iod;

    NODISCARD bool is_valid(ts::Glo const& time) const NOEXCEPT;
    NODISCARD bool match(GloEphemeris const& other) const NOEXCEPT {
        if (slot_number != other.slot_number) return false;
        if (std::fabs(reference_time.timestamp().full_seconds() -
                      other.reference_time.timestamp().full_seconds()) > 1e-3)
            return false;
        if (lpp_iod != other.lpp_iod) return false;
        if (health != other.health) return false;
        return true;
    }
    NODISCARD bool compare(GloEphemeris const& other) const NOEXCEPT {
        if (reference_time.timestamp() < other.reference_time.timestamp()) return true;
        if (reference_time.timestamp() > other.reference_time.timestamp()) return false;
        if (lpp_iod < other.lpp_iod) return true;
        if (lpp_iod > other.lpp_iod) return false;
        if (health < other.health) return true;
        if (health > other.health) return false;
        return false;
    }
    NODISCARD EphemerisResult compute(ts::Glo const& time) const NOEXCEPT;
    NODISCARD double          calculate_clock_bias(double dt) const NOEXCEPT;
    NODISCARD double          calculate_relativistic_correction(Float3 const& position,
                                                                Float3 const& velocity) const NOEXCEPT;

    MSGPACK_DEFINE(slot_number, frequency_number, reference_time, position, velocity, acceleration,
                   tau_n, gamma_n, health, age, m, lpp_iod)
};

}  // namespace ephemeris

namespace streamline {
template <typename T>
struct TypeName;

template <>
struct TypeName<ephemeris::GloEphemeris> {
    static char const* name() { return "ephemeris::GloEphemeris"; }
};
}  // namespace streamline
