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
    if (prn >= 1 && prn <= 63) {
        return SatelliteId(Gnss::BEIDOU, prn - 1);
    } else {
        return invalid();
    }
}

SatelliteId SatelliteId::from_qzs_prn(uint8_t prn) {
    if (prn >= 193 && prn <= 202) {
        return SatelliteId(Gnss::QZSS, prn - 193);
    } else {
        return invalid();
    }
}

SatelliteId SatelliteId::from_string(std::string const& str) {
    if (str.size() < 3) {
        return invalid();
    }

    Gnss gnss = Gnss::UNKNOWN;
    switch (str[0]) {
    case 'G': gnss = Gnss::GPS; break;
    case 'R': gnss = Gnss::GLONASS; break;
    case 'E': gnss = Gnss::GALILEO; break;
    case 'C': gnss = Gnss::BEIDOU; break;
    case 'J': gnss = Gnss::QZSS; break;
    default: return invalid();
    }

    try {
        char const* lpp_id_str = str.c_str() + 1;
        char*       end;
        long        lpp_id = strtol(lpp_id_str, &end, 10);
        if (end != str.c_str() + str.size()) {
            return invalid();
        } else if (lpp_id < 1) {
            return invalid();
        } else {
            return from_lpp(gnss, lpp_id - 1);
        }
    } catch (...) {
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

Maybe<uint8_t> SatelliteId::prn() const {
    if (mLppId >= 0 && mLppId < LPP_SATELLITE_ID_LIMIT) {
        return static_cast<uint8_t>(mLppId + 1);
    } else {
        return Maybe<uint8_t>();
    }
}

long SatelliteId::absolute_id() const {
    switch (mGnss) {
    case Gnss::GPS: return GPS_ABS_MIN + mLppId;
    case Gnss::GLONASS: return GLO_ABS_MIN + mLppId;
    case Gnss::GALILEO: return GAL_ABS_MIN + mLppId;
    case Gnss::BEIDOU: return BDS_ABS_MIN + mLppId;
    case Gnss::QZSS: return QZS_ABS_MIN + mLppId;
    case Gnss::UNKNOWN: return -1;
    }
    return -1;
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
    case Gnss::QZSS: snprintf(buffer, sizeof(buffer), "J%02" PRId32 "", mLppId); break;
    case Gnss::UNKNOWN: snprintf(buffer, sizeof(buffer), "U%02" PRId32 "", mLppId); break;
    }
    return std::string(buffer);
}

static char const* gGpsSatelliteNames[32] = {
    "G01", "G02", "G03", "G04", "G05", "G06", "G07", "G08", "G09", "G10", "G11",
    "G12", "G13", "G14", "G15", "G16", "G17", "G18", "G19", "G20", "G21", "G22",
    "G23", "G24", "G25", "G26", "G27", "G28", "G29", "G30", "G31", "G32",
};

static char const* gGloSatelliteNames[24] = {
    "R01", "R02", "R03", "R04", "R05", "R06", "R07", "R08", "R09", "R10", "R11", "R12",
    "R13", "R14", "R15", "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23", "R24",
};

static char const* gGalSatelliteNames[36] = {
    "E01", "E02", "E03", "E04", "E05", "E06", "E07", "E08", "E09", "E10", "E11", "E12",
    "E13", "E14", "E15", "E16", "E17", "E18", "E19", "E20", "E21", "E22", "E23", "E24",
    "E25", "E26", "E27", "E28", "E29", "E30", "E31", "E32", "E33", "E34", "E35", "E36",
};

static char const* gBdsSatelliteNames[63] = {
    "C01", "C02", "C03", "C04", "C05", "C06", "C07", "C08", "C09", "C10", "C11", "C12", "C13",
    "C14", "C15", "C16", "C17", "C18", "C19", "C20", "C21", "C22", "C23", "C24", "C25", "C26",
    "C27", "C28", "C29", "C30", "C31", "C32", "C33", "C34", "C35", "C36", "C37", "C38", "C39",
    "C40", "C41", "C42", "C43", "C44", "C45", "C46", "C47", "C48", "C49", "C50", "C51", "C52",
    "C53", "C54", "C55", "C56", "C57", "C58", "C59", "C60", "C61", "C62", "C63",
};

static char const* gQzsSatelliteNames[10] = {
    "J01", "J02", "J03", "J04", "J05", "J06", "J07", "J08", "J09", "J10",
};

char const* SatelliteId::name() const {
    if (!is_valid()) return "X--";
    switch (mGnss) {
    case Gnss::GPS: return (mLppId >= 0 && mLppId < 32) ? gGpsSatelliteNames[mLppId] : "G--";
    case Gnss::GLONASS: return (mLppId >= 0 && mLppId < 24) ? gGloSatelliteNames[mLppId] : "R--";
    case Gnss::GALILEO: return (mLppId >= 0 && mLppId < 36) ? gGalSatelliteNames[mLppId] : "E--";
    case Gnss::BEIDOU: return (mLppId >= 0 && mLppId < 63) ? gBdsSatelliteNames[mLppId] : "C--";
    case Gnss::QZSS: return (mLppId >= 0 && mLppId < 10) ? gQzsSatelliteNames[mLppId] : "J--";
    case Gnss::UNKNOWN: return "U--";
    }
    CORE_UNREACHABLE();
}
