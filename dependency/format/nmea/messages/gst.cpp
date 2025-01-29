#include "gst.hpp"
#include "helper.hpp"

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "nmea/gst"

namespace format {
namespace nmea {

static bool parse_double_opt(std::string const& token, double& value) {
    try {
        value = std::stod(token);
        return true;
    } catch (...) {
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
    // split payload by ','
    auto tokens = split(payload, ',');

    // check number of tokens
    if (tokens.size() < 8) {
        DEBUGF("invalid number of tokens: %zu", tokens.size());
        return nullptr;
    }

    // parse
    auto message = new GstMessage(prefix, payload, checksum);
    auto success = true;
    success &= parse_double_opt(tokens[1], message->mRmsValue);
    success &= parse_double_opt(tokens[2], message->mSemiMajorError);
    success &= parse_double_opt(tokens[3], message->mSemiMinorError);
    success &= parse_double_opt(tokens[4], message->mOrientationOfSemiMajorError);
    success &= parse_double_opt(tokens[5], message->mLatitudeError);
    success &= parse_double_opt(tokens[6], message->mLongitudeError);
    success &= parse_double_opt(tokens[7], message->mAltitudeError);

    if (success) {
        return std::unique_ptr<GstMessage>(message);
    } else {
        DEBUGF("failed to parse message");
        delete message;
        return nullptr;
    }
}

}  // namespace nmea
}  // namespace receiver
