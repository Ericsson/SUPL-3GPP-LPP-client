#include "gga.hpp"
#include "helper.hpp"

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

LOGLET_MODULE3(format, nmea, gga);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, nmea, gga)

namespace format {
namespace nmea {

// parse UTC time of day from string "hhmmss.sss"
static bool parse_utc(std::string const& utc, ts::Tai& time_of_day) {
    FUNCTION_SCOPEF("'%s'", utc.c_str());
    try {
        auto tokens = split(utc, '.');
        if (tokens.size() != 2) {
            VERBOSEF("invalid UTC format: expected 2 tokens, got %zu", tokens.size());
            return false;
        }

        auto hours        = std::stoi(tokens[0].substr(0, 2));
        auto minutes      = std::stoi(tokens[0].substr(2, 2));
        auto seconds      = std::stoi(tokens[0].substr(4, 2));
        auto milliseconds = std::stoi(tokens[1]);

        VERBOSEF("parsed time: %02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);

        auto tod = hours * ts::HOUR_IN_SECONDS + minutes * ts::MINUTE_IN_SECONDS + seconds +
                   milliseconds * 1e-3;
        auto utc_now  = ts::Utc::now();
        auto utc_then = ts::Utc::from_day_tod(utc_now.days(), tod);
        time_of_day   = ts::Tai{utc_then};
        return true;
    } catch (...) {
        VERBOSEF("exception parsing UTC: '%s'", utc.c_str());
        return false;
    }
}

// parse latitude from string "ddmm.mmmm*"
static bool parse_latitude(std::string const& latitude, std::string const& nw_indicator,
                           double& lat) {
    FUNCTION_SCOPEF("'%s' %s", latitude.c_str(), nw_indicator.c_str());
    try {
        auto degrees = std::stod(latitude.substr(0, 2));
        auto minutes = std::stod(latitude.substr(2));
        auto value   = degrees + minutes / 60.0;

        if (nw_indicator == "S") {
            lat = -value;
            VERBOSEF("latitude: %.8f (S)", lat);
            return true;
        } else if (nw_indicator == "N") {
            lat = value;
            VERBOSEF("latitude: %.8f (N)", lat);
            return true;
        } else {
            VERBOSEF("invalid N/S indicator: '%s'", nw_indicator.c_str());
            return false;
        }
    } catch (...) {
        VERBOSEF("exception parsing latitude: '%s'", latitude.c_str());
        return false;
    }
}

// parse longitude from string "dddmm.mmmm*"
static bool parse_longitude(std::string const& longitude, std::string const& ew_indicator,
                            double& lon) {
    FUNCTION_SCOPEF("'%s' %s", longitude.c_str(), ew_indicator.c_str());
    try {
        auto degrees = std::stod(longitude.substr(0, 3));
        auto minutes = std::stod(longitude.substr(3));
        auto value   = degrees + minutes / 60.0;

        if (ew_indicator == "W") {
            lon = -value;
            VERBOSEF("longitude: %.8f (W)", lon);
            return true;
        } else if (ew_indicator == "E") {
            lon = value;
            VERBOSEF("longitude: %.8f (E)", lon);
            return true;
        } else {
            VERBOSEF("invalid E/W indicator: '%s'", ew_indicator.c_str());
            return false;
        }
    } catch (...) {
        VERBOSEF("exception parsing longitude: '%s'", longitude.c_str());
        return false;
    }
}

static bool parse_fix_quality(std::string const& fix_quality, GgaFixQuality& quality) {
    FUNCTION_SCOPEF("'%s'", fix_quality.c_str());
    try {
        auto value = std::stoi(fix_quality);
        switch (value) {
        case 0: quality = GgaFixQuality::Invalid; break;
        case 1: quality = GgaFixQuality::GpsFix; break;
        case 2: quality = GgaFixQuality::DgpsFix; break;
        case 3: quality = GgaFixQuality::PpsFix; break;
        case 4: quality = GgaFixQuality::RtkFixed; break;
        case 5: quality = GgaFixQuality::RtkFloat; break;
        case 6: quality = GgaFixQuality::DeadReckoning; break;
        default: VERBOSEF("invalid fix quality value: %d", value); return false;
        }

        VERBOSEF("fix quality: %d", value);
        return true;
    } catch (...) {
        VERBOSEF("exception parsing fix quality: '%s'", fix_quality.c_str());
        return false;
    }
}

static bool parse_satellites_in_view(std::string const& satellites_in_view, int& satellites) {
    FUNCTION_SCOPEF("'%s'", satellites_in_view.c_str());
    try {
        satellites = std::stoi(satellites_in_view);
        VERBOSEF("satellites: %d", satellites);
        return true;
    } catch (...) {
        VERBOSEF("exception parsing satellites: '%s'", satellites_in_view.c_str());
        return false;
    }
}

static bool parse_hdop(std::string const& hdop, double& value) {
    FUNCTION_SCOPEF("'%s'", hdop.c_str());
    try {
        value = std::stod(hdop);
        VERBOSEF("hdop: %.4f", value);
        return true;
    } catch (...) {
        VERBOSEF("exception parsing hdop: '%s'", hdop.c_str());
        return false;
    }
}

static bool parse_altitude(std::string const& altitude, std::string const& units, double& value) {
    FUNCTION_SCOPEF("'%s' %s", altitude.c_str(), units.c_str());
    try {
        value = std::stod(altitude);
        if (units == "M") {
            VERBOSEF("altitude: %.2f m", value);
            return true;
        } else {
            VERBOSEF("invalid altitude units: '%s'", units.c_str());
            return false;
        }
    } catch (...) {
        VERBOSEF("exception parsing altitude: '%s'", altitude.c_str());
        return false;
    }
}

static bool
parse_age_of_differential_corrections(std::string const& age_of_differential_corrections,
                                      double&            value) {
    FUNCTION_SCOPEF("'%s'", age_of_differential_corrections.c_str());
    try {
        value = std::stod(age_of_differential_corrections);
        VERBOSEF("age: %.2f", value);
        return true;
    } catch (...) {
        VERBOSEF("exception parsing age: '%s'", age_of_differential_corrections.c_str());
        return false;
    }
}

GgaMessage::GgaMessage(std::string prefix, std::string payload, std::string checksum) NOEXCEPT
    : Message{prefix, payload, checksum},
      mTimeOfDay{},
      mLatitude{0.0},
      mLongitude{0.0},
      mFixQuality{GgaFixQuality::Invalid},
      mSatellitesInView{0},
      mHdop{0.0},
      mMsl{0.0},
      mGeoidSeparation{0.0},
      mAgeOfDifferentialCorrections{0} {}

void GgaMessage::print() const NOEXCEPT {
    printf("[%5s]\n", prefix().c_str());
    printf("  time of day: %s\n", time_of_day().rtklib_time_string().c_str());
    printf("  latitude:    %.8f\n", latitude());
    printf("  longitude:   %.8f\n", longitude());
    printf("  fix quality: ");
    switch (fix_quality()) {
    case GgaFixQuality::Invalid: printf("invalid"); break;
    case GgaFixQuality::GpsFix: printf("gps fix"); break;
    case GgaFixQuality::DgpsFix: printf("dgps fix"); break;
    case GgaFixQuality::PpsFix: printf("pps fix"); break;
    case GgaFixQuality::RtkFixed: printf("rtk fixed"); break;
    case GgaFixQuality::RtkFloat: printf("rtk float"); break;
    case GgaFixQuality::DeadReckoning: printf("dead reckoning"); break;
    }
    printf(" (%d)\n", static_cast<int>(fix_quality()));
    printf("  satellites:  %d\n", satellites_in_view());
    printf("  hdop:        %.4f\n", h_dop());
    printf("  altitude:    %.2f\n", altitude());
    printf("  age:         %.2f\n", age_of_differential_corrections());
}

std::unique_ptr<Message> GgaMessage::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new GgaMessage(*this));
}

std::unique_ptr<Message> GgaMessage::parse(std::string prefix, std::string const& payload,
                                           std::string checksum) {
    FUNCTION_SCOPEF("%s,%s*%s", prefix.c_str(), payload.c_str(), checksum.c_str());
    auto tokens = split(payload, ',');

    if (tokens.size() < 13) {
        VERBOSEF("invalid token count: %zu (expected >= 13)", tokens.size());
        return nullptr;
    }

    VERBOSEF("token count: %zu", tokens.size());

    // parse
    auto message = new GgaMessage(prefix, payload, checksum);
    auto success = true;
    success &= parse_utc(tokens[0], message->mTimeOfDay);
    success &= parse_latitude(tokens[1], tokens[2], message->mLatitude);
    success &= parse_longitude(tokens[3], tokens[4], message->mLongitude);
    success &= parse_fix_quality(tokens[5], message->mFixQuality);
    success &= parse_satellites_in_view(tokens[6], message->mSatellitesInView);
    success &= parse_hdop(tokens[7], message->mHdop);
    success &= parse_altitude(tokens[8], tokens[9], message->mMsl);
    success &= parse_altitude(tokens[10], tokens[11], message->mGeoidSeparation);

    if (tokens.size() > 12) {
        if (!parse_age_of_differential_corrections(tokens[12],
                                                   message->mAgeOfDifferentialCorrections)) {
            message->mAgeOfDifferentialCorrections = 0;
        }
    } else {
        message->mAgeOfDifferentialCorrections = 0;
    }

    if (success) {
        TRACEF("GGA message parsed successfully");
        return std::unique_ptr<GgaMessage>(message);
    } else {
        VERBOSEF("failed to parse GGA message");
        delete message;
        return nullptr;
    }
}

}  // namespace nmea
}  // namespace format
