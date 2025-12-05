#pragma once
#include <core/core.hpp>
#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <ephemeris/qzs.hpp>
#include <ephemeris/result.hpp>
#include <maths/float3.hpp>
#include <time/tai.hpp>

namespace msgpack {
class Packer;
class Unpacker;
}  // namespace msgpack

namespace ephemeris {

struct Ephemeris {
public:
    enum class Type {
        NONE,
        GPS,
        GAL,
        BDS,
        QZS,
    };

    Ephemeris() NOEXCEPT : mType(Type::NONE) {}
    EXPLICIT Ephemeris(GpsEphemeris const& ephemeris) NOEXCEPT : mType(Type::GPS),
                                                                 gps_ephemeris(ephemeris) {}
    EXPLICIT Ephemeris(GalEphemeris const& ephemeris) NOEXCEPT : mType(Type::GAL),
                                                                 gal_ephemeris(ephemeris) {}
    EXPLICIT Ephemeris(BdsEphemeris const& ephemeris) NOEXCEPT : mType(Type::BDS),
                                                                 bds_ephemeris(ephemeris) {}
    EXPLICIT Ephemeris(QzsEphemeris const& ephemeris) NOEXCEPT : mType(Type::QZS),
                                                                 qzs_ephemeris(ephemeris) {}

    /// Issue of Data as defined by 3GPP LPP
    NODISCARD uint16_t iod() const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return 0;
        case Type::GPS: return gps_ephemeris.lpp_iod;
        case Type::GAL: return gal_ephemeris.lpp_iod;
        case Type::BDS: return bds_ephemeris.lpp_iod;
        case Type::QZS: return qzs_ephemeris.lpp_iod;
        }
        CORE_UNREACHABLE();
    }

    NODISCARD uint16_t iode() const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return 0;
        case Type::GPS: return gps_ephemeris.iode;
        case Type::GAL: return gal_ephemeris.iod_nav;
        case Type::BDS: return bds_ephemeris.iode;
        case Type::QZS: return qzs_ephemeris.iode;
        }
        CORE_UNREACHABLE();
    }

    NODISCARD uint16_t iodc() const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return 0;
        case Type::GPS: return gps_ephemeris.iodc;
        case Type::GAL: return gal_ephemeris.iod_nav;
        case Type::BDS: return bds_ephemeris.iodc;
        case Type::QZS: return qzs_ephemeris.iodc;
        }
        CORE_UNREACHABLE();
    }

    NODISCARD bool is_valid(ts::Tai const& time) const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return false;
        case Type::GPS: return gps_ephemeris.is_valid(ts::Gps{time});
        case Type::GAL: return gal_ephemeris.is_valid(ts::Gst{time});
        case Type::BDS: return bds_ephemeris.is_valid(ts::Bdt{time});
        case Type::QZS: return qzs_ephemeris.is_valid(ts::Gps{time});
        }
        CORE_UNREACHABLE();
    }

    NODISCARD EphemerisResult compute(ts::Tai const& time) const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return EphemerisResult{};
        case Type::GPS: return gps_ephemeris.compute(ts::Gps{time});
        case Type::GAL: return gal_ephemeris.compute(ts::Gst{time});
        case Type::BDS: return bds_ephemeris.compute(ts::Bdt{time});
        case Type::QZS: return qzs_ephemeris.compute(ts::Gps{time});
        }
        CORE_UNREACHABLE();
    }

    NODISCARD double clock_bias(ts::Tai const& time) const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return 0.0;
        case Type::GPS: return gps_ephemeris.calculate_clock_bias(ts::Gps{time});
        case Type::GAL: return gal_ephemeris.calculate_clock_bias(ts::Gst{time});
        case Type::BDS: return bds_ephemeris.calculate_clock_bias(ts::Bdt{time});
        case Type::QZS: return qzs_ephemeris.calculate_clock_bias(ts::Gps{time});
        }
        CORE_UNREACHABLE();
    }

    NODISCARD double relativistic_correction(Float3 const& position,
                                             Float3 const& velocity) const NOEXCEPT {
        switch (mType) {
        case Type::NONE: return 0.0;
        case Type::GPS: return gps_ephemeris.calculate_relativistic_correction(position, velocity);
        case Type::GAL: return gal_ephemeris.calculate_relativistic_correction(position, velocity);
        case Type::BDS: return bds_ephemeris.calculate_relativistic_correction(position, velocity);
        case Type::QZS: return qzs_ephemeris.calculate_relativistic_correction(position, velocity);
        }
        CORE_UNREACHABLE();
    }

    friend void msgpack::pack(msgpack::Packer&, ephemeris::Ephemeris const&) NOEXCEPT;
    friend bool msgpack::unpack(msgpack::Unpacker&, ephemeris::Ephemeris&) NOEXCEPT;

    void msgpack_pack(msgpack::Packer& packer) const NOEXCEPT;
    bool msgpack_unpack(msgpack::Unpacker& unpacker) NOEXCEPT;

    Type mType;
    union {
        GpsEphemeris gps_ephemeris;
        GalEphemeris gal_ephemeris;
        BdsEphemeris bds_ephemeris;
        QzsEphemeris qzs_ephemeris;
    };
};

}  // namespace ephemeris
