#include <receiver/nmea/gga.hpp>
#include "helper.hpp"

namespace receiver {
namespace nmea {

// parse UTC time of day from string "hhmmss.sss"
static bool parse_utc(const std::string& utc, TAI_Time& time_of_day) {
    try {
        auto tokens = split(utc, '.');
        if (tokens.size() != 2) {
            return false;
        }

        auto hours        = std::stoi(tokens[0].substr(0, 2));
        auto minutes      = std::stoi(tokens[0].substr(2, 2));
        auto seconds      = std::stoi(tokens[0].substr(4, 2));
        auto milliseconds = std::stoi(tokens[1]);

        auto tod =
            hours * HOUR_IN_SECONDS + minutes * MINUTE_IN_SECONDS + seconds + milliseconds * 1e-3;
        auto utc_now  = UTC_Time::now();
        auto utc_then = UTC_Time{utc_now.days(), tod};
        time_of_day   = TAI_Time{utc_then};
        return true;
    } catch (...) {
        return false;
    }
}

// parse latitude from string "ddmm.mmmm*"
static bool parse_latitude(const std::string& latitude, const std::string& nw_indicator,
                           double& lat) {
    try {
        auto degrees = std::stod(latitude.substr(0, 2));
        auto minutes = std::stod(latitude.substr(2));
        auto value   = degrees + minutes / 60.0;

        if (nw_indicator == "S") {
            lat = -value;
            return true;
        } else if (nw_indicator == "N") {
            lat = value;
            return true;
        } else {
            return false;
        }
    } catch (...) {
        return false;
    }
}

// parse longitude from string "dddmm.mmmm*"
static bool parse_longitude(const std::string& longitude, const std::string& ew_indicator,
                            double& lon) {
    try {
        auto degrees = std::stod(longitude.substr(0, 3));
        auto minutes = std::stod(longitude.substr(3));
        auto value   = degrees + minutes / 60.0;

        if (ew_indicator == "W") {
            lon = -value;
            return true;
        } else if (ew_indicator == "E") {
            lon = value;
            return true;
        } else {
            return false;
        }
    } catch (...) {
        return false;
    }
}

static bool parse_fix_quality(const std::string& fix_quality, GgaFixQuality& quality) {
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
        default: return false;
        }

        return true;
    } catch (...) {
        return false;
    }
}

static bool parse_satellites_in_view(const std::string& satellites_in_view, int& satellites) {
    try {
        satellites = std::stoi(satellites_in_view);
        return true;
    } catch (...) {
        return false;
    }
}

static bool parse_hdop(const std::string& hdop, double& value) {
    try {
        value = std::stod(hdop);
        return true;
    } catch (...) {
        return false;
    }
}

static bool parse_altitude(const std::string& altitude, const std::string& units, double& value) {
    try {
        value = std::stod(altitude);
        if (units == "M") {
            return true;
        } else {
            return false;
        }
    } catch (...) {
        return false;
    }
}

GgaMessage::GgaMessage(std::string prefix, std::string payload, std::string checksum) NMEA_NOEXCEPT
    : Message{prefix, payload, checksum},
      mTimeOfDay{TAI_Time::now()},
      mLatitude{0.0},
      mLongitude{0.0},
      mFixQuality{GgaFixQuality::Invalid},
      mSatellitesInView{0} {}

void GgaMessage::print() const NMEA_NOEXCEPT {
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
}

std::unique_ptr<Message> GgaMessage::parse(std::string prefix, const std::string& payload,
                                           std::string checksum) {
    // split payload by ','
    auto tokens = split(payload, ',');

    // check number of tokens
    if (tokens.size() < 13) {
        return nullptr;
    }

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

    if (success) {
        return std::unique_ptr<GgaMessage>(message);
    } else {
        delete message;
        return nullptr;
    }
}

}  // namespace nmea
}  // namespace receiver
