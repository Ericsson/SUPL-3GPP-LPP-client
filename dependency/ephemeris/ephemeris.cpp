#include <ephemeris/ephemeris.hpp>
#include <msgpack/msgpack.hpp>

namespace ephemeris {

void Ephemeris::msgpack_pack(msgpack::Packer& packer) const NOEXCEPT {
    packer.pack_array_header(2);
    msgpack::pack(packer, static_cast<int32_t>(mType));

    if (mType == Type::GPS) {
        msgpack::pack(packer, gps_ephemeris);
    } else if (mType == Type::GAL) {
        msgpack::pack(packer, gal_ephemeris);
    } else if (mType == Type::BDS) {
        msgpack::pack(packer, bds_ephemeris);
    } else if (mType == Type::QZS) {
        msgpack::pack(packer, qzs_ephemeris);
    } else {
        packer.pack_nil();
    }
}

bool Ephemeris::msgpack_unpack(msgpack::Unpacker& unpacker) NOEXCEPT {
    uint32_t size = 0;
    if (!unpacker.unpack_array_header(size)) return false;
    if (size != 2) return false;

    int32_t type_int;
    if (!msgpack::unpack(unpacker, type_int)) return false;

    mType = static_cast<Type>(type_int);
    if (mType == Type::GPS) {
        GpsEphemeris eph;
        if (!msgpack::unpack(unpacker, eph)) return false;
        gps_ephemeris = eph;
    } else if (mType == Type::GAL) {
        GalEphemeris eph;
        if (!msgpack::unpack(unpacker, eph)) return false;
        gal_ephemeris = eph;
    } else if (mType == Type::BDS) {
        BdsEphemeris eph;
        if (!msgpack::unpack(unpacker, eph)) return false;
        bds_ephemeris = eph;
    } else if (mType == Type::QZS) {
        QzsEphemeris eph;
        if (!msgpack::unpack(unpacker, eph)) return false;
        qzs_ephemeris = eph;
    } else {
        return false;
    }

    return true;
}

}  // namespace ephemeris
