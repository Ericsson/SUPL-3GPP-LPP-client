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

    /// Issue of Data as defined by 3GPP LPP
    NODISCARD uint16_t iod() const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return 0;
        case Type::GPS: return mGpsEphemeris.lpp_iod;
        case Type::GAL: return mGalEphemeris.lpp_iod;
        case Type::BDS: return mBdsEphemeris.lpp_iod;
        }
    }

    NODISCARD uint16_t iode() const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return 0;
        case Type::GPS: return mGpsEphemeris.iode;
        case Type::GAL: return mGalEphemeris.iod_nav;
        case Type::BDS: return mBdsEphemeris.iode;
        }
    }

    NODISCARD uint16_t iodc() const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return 0;
        case Type::GPS: return mGpsEphemeris.iodc;
        case Type::GAL: return mGalEphemeris.iod_nav;
        case Type::BDS: return mBdsEphemeris.iodc;
        }
    }

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

    NODISCARD double relativistic_correction(Float3 const& position,
                                             Float3 const& velocity) const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return 0.0;
        case Type::GPS: return mGpsEphemeris.calculate_relativistic_correction(position, velocity);
        case Type::GAL: return mGalEphemeris.calculate_relativistic_correction(position, velocity);
        case Type::BDS: return mBdsEphemeris.calculate_relativistic_correction(position, velocity);
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
