#include <receiver/nmea/vtg.hpp>
#include "helper.hpp"

namespace receiver {
namespace nmea {

static bool parse_double(const std::string& token, double& value) {
    try {
        value = std::stod(token);
        return true;
    } catch (...) {
#if RECEIVER_NMEA_DEBUG
        printf("[--VTG] failed to parse double: \"%s\"\n", token.c_str());
#endif
        return false;
    }
}

static bool parse_double_opt(const std::string& token, double& value) {
    try {
        value = std::stod(token);
        return true;
    } catch (...) {
        value = 0;
        return true;
    }
}

static bool parse_mode_indicator(const std::string& token, ModeIndicator& mode_indicator) {
    if (token.size() != 1) {
#if RECEIVER_NMEA_DEBUG
        printf("[--VTG] invalid mode indicator: \"%s\"\n", token.c_str());
#endif
        return false;
    }

    switch (token[0]) {
    case 'A': mode_indicator = ModeIndicator::Autonomous; return true;
    case 'D': mode_indicator = ModeIndicator::Differential; return true;
    default: mode_indicator = ModeIndicator::Unknown; return true;
    }
}

VtgMessage::VtgMessage(std::string prefix, std::string payload, std::string checksum) NMEA_NOEXCEPT
    : Message{prefix, payload, checksum},
      mTrueCourseOverGround{0.0},
      mMagneticCourseOverGround{0.0},
      mSpeedOverGroundKnots{0.0},
      mSpeedOverGroundKmh{0.0},
      mModeIndicator{ModeIndicator::Unknown} {}

void VtgMessage::print() const NMEA_NOEXCEPT {
    printf("[%5s]\n", prefix().c_str());
    printf("  course over ground (true): %.2f\n", mTrueCourseOverGround);
    printf("  course over ground (magnetic): %.2f\n", mMagneticCourseOverGround);
    printf("  speed over ground (knots): %.2f\n", mSpeedOverGroundKnots);
    printf("  speed over ground (km/h): %.2f\n", mSpeedOverGroundKmh);
    printf("  mode indicator: ");
    switch (mModeIndicator) {
    case ModeIndicator::Unknown: printf("unknown\n"); break;
    case ModeIndicator::Autonomous: printf("autonomous\n"); break;
    case ModeIndicator::Differential: printf("differential\n"); break;
    default: printf("invalid\n"); break;
    }
}

std::unique_ptr<Message> VtgMessage::parse(std::string prefix, const std::string& payload,
                                           std::string checksum) {
    // split payload by ','
    auto tokens = split(payload, ',');

    // check number of tokens
    if (tokens.size() < 9) {
#if RECEIVER_NMEA_DEBUG
        printf("[--VTG] invalid number of tokens: %zu\n", tokens.size());
#endif
        return nullptr;
    }

    // parse
    auto message = new VtgMessage(prefix, payload, checksum);
    auto success = true;
    success &= parse_double_opt(tokens[0], message->mTrueCourseOverGround);
    success &= parse_double_opt(tokens[2], message->mMagneticCourseOverGround);
    success &= parse_double(tokens[4], message->mSpeedOverGroundKnots);
    success &= parse_double(tokens[6], message->mSpeedOverGroundKmh);
    success &= tokens[7] == "K";
    success &= parse_mode_indicator(tokens[8], message->mModeIndicator);

    if (success) {
        return std::unique_ptr<VtgMessage>(message);
    } else {
#if RECEIVER_NMEA_DEBUG
        printf("[--VTG] failed to parse message\n");
#endif
        delete message;
        return nullptr;
    }
}

}  // namespace nmea
}  // namespace receiver
