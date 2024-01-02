#include <receiver/nmea/gpgga.hpp>

#include <sstream>
#include <string>
#include <vector>

static std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::string              token;
    std::istringstream       token_stream(str);
    while (std::getline(token_stream, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

namespace receiver {
namespace nmea {

// parse UTC time of day from string "hhmmss.sss"
static bool parse_utc(const std::string& utc, TAI_Time& time_of_day) {
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
}

// parse latitude from string "ddmm.mmmm*"
static bool parse_latitude(const std::string& latitude, const std::string& nw_indicator,
                           double& lat) {
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
}

// parse longitude from string "dddmm.mmmm*"
static bool parse_longitude(const std::string& longitude, const std::string& ew_indicator,
                            double& lon) {
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
}

static bool parse_fix_quality(const std::string& fix_quality, GpggaFixQuality& quality) {
    auto value = std::stoi(fix_quality);
    switch (value) {
    case 0: quality = GpggaFixQuality::Invalid; break;
    case 1: quality = GpggaFixQuality::GpsFix; break;
    case 2: quality = GpggaFixQuality::DgpsFix; break;
    case 3: quality = GpggaFixQuality::PpsFix; break;
    case 4: quality = GpggaFixQuality::RtkFixed; break;
    case 5: quality = GpggaFixQuality::RtkFloat; break;
    case 6: quality = GpggaFixQuality::DeadReckoning; break;
    default: return false;
    }

    return true;
}

static bool parse_satellites_in_view(const std::string& satellites_in_view, int& satellites) {
    satellites = std::stoi(satellites_in_view);
    return true;
}

GpggaMessage::GpggaMessage() NMEA_NOEXCEPT : Message{"$GPGGA"},
                                             mTimeOfDay{TAI_Time::now()},
                                             mLatitude{0.0},
                                             mLongitude{0.0},
                                             mFixQuality{GpggaFixQuality::Invalid},
                                             mSatellitesInView{0} {}

void GpggaMessage::print() const NMEA_NOEXCEPT {
    printf("[%6s]\n", prefix().c_str());
    printf("  time of day: %s\n", time_of_day().rtklib_time_string().c_str());
    printf("  latitude:    %.8f\n", latitude());
    printf("  longitude:   %.8f\n", longitude());
    printf("  fix quality: ");
    switch (fix_quality()) {
    case GpggaFixQuality::Invalid: printf("invalid"); break;
    case GpggaFixQuality::GpsFix: printf("gps fix"); break;
    case GpggaFixQuality::DgpsFix: printf("dgps fix"); break;
    case GpggaFixQuality::PpsFix: printf("pps fix"); break;
    case GpggaFixQuality::RtkFixed: printf("rtk fixed"); break;
    case GpggaFixQuality::RtkFloat: printf("rtk float"); break;
    case GpggaFixQuality::DeadReckoning: printf("dead reckoning"); break;
    }
    printf(" (%d)\n", static_cast<int>(fix_quality()));
    printf("  satellites:  %d\n", satellites_in_view());
}

std::unique_ptr<GpggaMessage> GpggaMessage::parse(const std::string& payload) {
    // split paylaod by ','
    auto tokens = split(payload, ',');

    // check number of tokens
    if (tokens.size() != 15) {
        return nullptr;
    }

    // check prefix
    if (tokens[0] != "$GPGGA") {
        return nullptr;
    }

    // parse
    auto message = std::unique_ptr<GpggaMessage>(new GpggaMessage());
    auto success = true;
    success &= parse_utc(tokens[1], message->mTimeOfDay);
    success &= parse_latitude(tokens[2], tokens[3], message->mLatitude);
    success &= parse_longitude(tokens[4], tokens[5], message->mLongitude);
    success &= parse_fix_quality(tokens[6], message->mFixQuality);
    success &= parse_satellites_in_view(tokens[7], message->mSatellitesInView);

    if (success) {
        return message;
    } else {
        return nullptr;
    }
}

}  // namespace nmea
}  // namespace receiver
