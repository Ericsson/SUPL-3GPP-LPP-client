#include "satellite_id.hpp"
#include <cinttypes>

CONSTEXPR static long LPP_SATELLITE_ID_LIMIT = 64;

SatelliteId SatelliteId::invalid() {
    return SatelliteId();
}

SatelliteId SatelliteId::from_lpp(Gnss gnss, long id) {
    if (id >= LPP_SATELLITE_ID_LIMIT) {
        return invalid();
    } else {
        return SatelliteId(gnss, static_cast<int32_t>(id));
    }
}

SatelliteId SatelliteId::from_gps_prn(uint8_t prn) {
    if (prn >= 1 && prn <= 32) {
        return SatelliteId(Gnss::GPS, prn - 1);
    } else {
        return invalid();
    }
}

SatelliteId SatelliteId::from_gal_prn(uint8_t prn) {
    if (prn >= 1 && prn <= 36) {
        return SatelliteId(Gnss::GALILEO, prn - 1);
    } else {
        return invalid();
    }
}

SatelliteId SatelliteId::from_bds_prn(uint8_t prn) {
    if (prn >= 1 && prn <= 35) {
        return SatelliteId(Gnss::BEIDOU, prn - 1);
    } else {
        return invalid();
    }
}

Maybe<long> SatelliteId::lpp_id() const {
    if (mLppId >= 0 && mLppId < LPP_SATELLITE_ID_LIMIT) {
        return mLppId;
    } else {
        return Maybe<long>();
    }
}

Maybe<long> SatelliteId::as_df009() const {
    if (mGnss == Gnss::GPS && mLppId >= 0 && mLppId < LPP_SATELLITE_ID_LIMIT) {
        return mLppId + 1;
    } else {
        return Maybe<long>();
    }
}

Maybe<long> SatelliteId::as_df038() const {
    if (mGnss == Gnss::GLONASS && mLppId >= 0 && mLppId < LPP_SATELLITE_ID_LIMIT) {
        return mLppId + 1;
    } else {
        return Maybe<long>();
    }
}

Maybe<long> SatelliteId::as_msm() const {
    if (mLppId >= 0 && mLppId < LPP_SATELLITE_ID_LIMIT) {
        return mLppId + 1;
    } else {
        return Maybe<long>();
    }
}

SatelliteId::Gnss SatelliteId::gnss() const {
    return mGnss;
}

std::string SatelliteId::to_string() const {
    char buffer[32];
    switch (mGnss) {
    case Gnss::GPS: snprintf(buffer, sizeof(buffer), "G%02" PRId32 "", mLppId); break;
    case Gnss::GLONASS: snprintf(buffer, sizeof(buffer), "R%02" PRId32 "", mLppId); break;
    case Gnss::GALILEO: snprintf(buffer, sizeof(buffer), "E%02" PRId32 "", mLppId); break;
    case Gnss::BEIDOU: snprintf(buffer, sizeof(buffer), "C%02" PRId32 "", mLppId); break;
    case Gnss::UNKNOWN: snprintf(buffer, sizeof(buffer), "U%02" PRId32 "", mLppId); break;
    }
    return std::string(buffer);
}

static char const* GPS_NAMES[32] = {
    "G01", "G02", "G03", "G04", "G05", "G06", "G07", "G08", "G09", "G10", "G11",
    "G12", "G13", "G14", "G15", "G16", "G17", "G18", "G19", "G20", "G21", "G22",
    "G23", "G24", "G25", "G26", "G27", "G28", "G29", "G30", "G31", "G32",
};

static char const* GLO_NAMES[24] = {
    "R01", "R02", "R03", "R04", "R05", "R06", "R07", "R08", "R09", "R10", "R11", "R12",
    "R13", "R14", "R15", "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23", "R24",
};

static char const* GAL_NAMES[36] = {
    "E01", "E02", "E03", "E04", "E05", "E06", "E07", "E08", "E09", "E10", "E11", "E12",
    "E13", "E14", "E15", "E16", "E17", "E18", "E19", "E20", "E21", "E22", "E23", "E24",
    "E25", "E26", "E27", "E28", "E29", "E30", "E31", "E32", "E33", "E34", "E35", "E36",
};

static char const* BDS_NAMES[35] = {
    "C01", "C02", "C03", "C04", "C05", "C06", "C07", "C08", "C09", "C10", "C11", "C12",
    "C13", "C14", "C15", "C16", "C17", "C18", "C19", "C20", "C21", "C22", "C23", "C24",
    "C25", "C26", "C27", "C28", "C29", "C30", "C31", "C32", "C33", "C34", "C35",
};

char const* SatelliteId::name() const {
    if (!is_valid()) return "X--";
    switch (mGnss) {
    case Gnss::GPS: return (mLppId >= 0 && mLppId < 32) ? GPS_NAMES[mLppId] : "G--";
    case Gnss::GLONASS: return (mLppId >= 0 && mLppId < 24) ? GLO_NAMES[mLppId] : "R--";
    case Gnss::GALILEO: return (mLppId >= 0 && mLppId < 36) ? GAL_NAMES[mLppId] : "E--";
    case Gnss::BEIDOU: return (mLppId >= 0 && mLppId < 35) ? BDS_NAMES[mLppId] : "C--";
    case Gnss::UNKNOWN: return "U--";
    }
}
