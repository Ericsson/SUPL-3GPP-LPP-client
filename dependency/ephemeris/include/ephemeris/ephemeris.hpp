#pragma once
#include <core/core.hpp>
#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <ephemeris/result.hpp>
#include <maths/float3.hpp>
#include <time/tai.hpp>

namespace ephemeris {

struct Ephemeris {
public:
    enum class Type {
        NONE,
        GPS,
        GAL,
        BDS,
    };

    Ephemeris() NOEXCEPT : mType(Type::NONE) {}
    EXPLICIT Ephemeris(GpsEphemeris const& ephemeris) NOEXCEPT : mType(Type::GPS),
                                                                 mGpsEphemeris(ephemeris) {}
    EXPLICIT Ephemeris(GalEphemeris const& ephemeris) NOEXCEPT : mType(Type::GAL),
                                                                 mGalEphemeris(ephemeris) {}
    EXPLICIT Ephemeris(BdsEphemeris const& ephemeris) NOEXCEPT : mType(Type::BDS),
                                                                 mBdsEphemeris(ephemeris) {}

    NODISCARD bool is_valid(ts::Tai const& time) const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return false;
        case Type::GPS: return mGpsEphemeris.is_valid(ts::Gps{time});
        case Type::GAL: return mGalEphemeris.is_valid(ts::Gst{time});
        case Type::BDS: return mBdsEphemeris.is_valid(ts::Bdt{time});
        }
    }

    NODISCARD EphemerisResult compute(ts::Tai const& time) const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return EphemerisResult{};
        case Type::GPS: return mGpsEphemeris.compute(ts::Gps{time});
        case Type::GAL: return mGalEphemeris.compute(ts::Gst{time});
        case Type::BDS: return mBdsEphemeris.compute(ts::Bdt{time});
        }
    }

private:
    Type mType;
    union {
        GpsEphemeris mGpsEphemeris;
        GalEphemeris mGalEphemeris;
        BdsEphemeris mBdsEphemeris;
    };
};

}  // namespace ephemeris
