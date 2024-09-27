#include "vtg.hpp"
#include "helper.hpp"

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "nmea/vtg"

namespace format {
namespace nmea {

static bool parse_double(std::string const& token, double& value) {
    try {
        value = std::stod(token);
        return true;
    } catch (...) {
        WARNF("failed to parse double: \"%s\"", token.c_str());
        return false;
    }
}

static bool parse_double_opt(std::string const& token, double& value) {
    try {
        value = std::stod(token);
        return true;
    } catch (...) {
        value = 0;
        return true;
    }
}

static bool parse_mode_indicator(std::string const& token, ModeIndicator& mode_indicator) {
    if (token.size() != 1) {
        WARNF("invalid mode indicator: \"%s\"", token.c_str());
        return false;
    }

    switch (token[0]) {
    case 'A': mode_indicator = ModeIndicator::Autonomous; return true;
    case 'D': mode_indicator = ModeIndicator::Differential; return true;
    default: mode_indicator = ModeIndicator::Unknown; return true;
    }
}

VtgMessage::VtgMessage(std::string prefix, std::string payload, std::string checksum) NOEXCEPT
    : Message{prefix, payload, checksum},
      mTrueCourseOverGround{0.0},
      mMagneticCourseOverGround{0.0},
      mSpeedOverGroundKnots{0.0},
      mSpeedOverGroundKmh{0.0},
      mModeIndicator{ModeIndicator::Unknown} {}

void VtgMessage::print() const NOEXCEPT {
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
    }
}

std::unique_ptr<Message> VtgMessage::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new VtgMessage(*this));
}

std::unique_ptr<Message> VtgMessage::parse(std::string prefix, std::string const& payload,
                                           std::string checksum) {
    // split payload by ','
    auto tokens = split(payload, ',');

    // check number of tokens
    if (tokens.size() < 9) {
        WARNF("invalid number of tokens: %zu", tokens.size());
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
        WARNF("failed to parse message");
        delete message;
        return nullptr;
    }
}

}  // namespace nmea
}  // namespace format
