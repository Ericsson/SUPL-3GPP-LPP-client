#include "gst.hpp"
#include "helper.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE3(format, nmea, gst);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, nmea, gst)

namespace format {
namespace nmea {

static bool parse_double(std::string const& token, double& value) {
    FUNCTION_SCOPEF("'%s'", token.c_str());
    try {
        value = std::stod(token);
        VERBOSEF("parsed: %.6f", value);
        return true;
    } catch (...) {
        VERBOSEF("exception parsing double: '%s'", token.c_str());
        return false;
    }
}

static bool parse_double_opt(std::string const& token, double& value) {
    FUNCTION_SCOPEF("'%s'", token.c_str());
    try {
        value = std::stod(token);
        VERBOSEF("parsed: %.6f", value);
        return true;
    } catch (...) {
        VERBOSEF("failed to parse, using default: '%s'", token.c_str());
        value = 0;
        return true;
    }
}

GstMessage::GstMessage(std::string prefix, std::string payload, std::string checksum) NOEXCEPT
    : Message{prefix, payload, checksum},
      mRmsValue{0.0},
      mSemiMajorError{0.0},
      mSemiMinorError{0.0},
      mOrientationOfSemiMajorError{0.0},
      mLatitudeError{0.0},
      mLongitudeError{0.0},
      mAltitudeError{0.0} {}

void GstMessage::print() const NOEXCEPT {
    printf("[%5s]\n", prefix().c_str());
    printf("  rms value: %f\n", mRmsValue);
    printf("  semi-major error: %f\n", mSemiMajorError);
    printf("  semi-minor error: %f\n", mSemiMinorError);
    printf("  orientation of semi-major error: %f\n", mOrientationOfSemiMajorError);
    printf("  latitude error: %f\n", mLatitudeError);
    printf("  longitude error: %f\n", mLongitudeError);
    printf("  altitude error: %f\n", mAltitudeError);
}

std::unique_ptr<Message> GstMessage::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new GstMessage(*this));
}

std::unique_ptr<Message> GstMessage::parse(std::string prefix, std::string const& payload,
                                           std::string checksum) {
    FUNCTION_SCOPEF("%s,%s*%s", prefix.c_str(), payload.c_str(), checksum.c_str());
    auto tokens = split(payload, ',');

    if (tokens.size() < 8) {
        VERBOSEF("invalid token count: %zu (expected >= 8)", tokens.size());
        return nullptr;
    }

    VERBOSEF("token count: %zu", tokens.size());

    // parse
    auto message = new GstMessage(prefix, payload, checksum);
    auto success = true;
    success &= parse_double_opt(tokens[1], message->mRmsValue);
    success &= parse_double(tokens[2], message->mSemiMajorError);
    success &= parse_double(tokens[3], message->mSemiMinorError);
    success &= parse_double_opt(tokens[4], message->mOrientationOfSemiMajorError);
    success &= parse_double(tokens[5], message->mLatitudeError);
    success &= parse_double(tokens[6], message->mLongitudeError);
    success &= parse_double(tokens[7], message->mAltitudeError);

    if (success) {
        TRACEF("GST message parsed successfully");
        return std::unique_ptr<GstMessage>(message);
    } else {
        VERBOSEF("failed to parse GST message");
        delete message;
        return nullptr;
    }
}

}  // namespace nmea
}  // namespace format
