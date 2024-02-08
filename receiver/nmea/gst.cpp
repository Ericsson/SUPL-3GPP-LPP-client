#include <receiver/nmea/gst.hpp>
#include "helper.hpp"

namespace receiver {
namespace nmea {

static bool parse_double(const std::string& token, double& value) {
    try {
        value = std::stod(token);
        return true;
    } catch (...) {
#if RECEIVER_NMEA_DEBUG
        printf("[--GST] failed to parse double: \"%s\"\n", token.c_str());
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

GstMessage::GstMessage(std::string prefix, std::string payload, std::string checksum) NMEA_NOEXCEPT
    : Message{prefix, payload, checksum},
      mRmsValue{0.0},
      mSemiMajorError{0.0},
      mSemiMinorError{0.0},
      mOrientationOfSemiMajorError{0.0},
      mLatitudeError{0.0},
      mLongitudeError{0.0},
      mAltitudeError{0.0} {}
void GstMessage::print() const NMEA_NOEXCEPT {
    printf("[%5s]\n", prefix().c_str());
    printf("  rms value: %f\n", mRmsValue);
    printf("  semi-major error: %f\n", mSemiMajorError);
    printf("  semi-minor error: %f\n", mSemiMinorError);
    printf("  orientation of semi-major error: %f\n", mOrientationOfSemiMajorError);
    printf("  latitude error: %f\n", mLatitudeError);
    printf("  longitude error: %f\n", mLongitudeError);
    printf("  altitude error: %f\n", mAltitudeError);
}

std::unique_ptr<Message> GstMessage::parse(std::string prefix, const std::string& payload,
                                           std::string checksum) {
    // split payload by ','
    auto tokens = split(payload, ',');

    // check number of tokens
    if (tokens.size() < 8) {
#if RECEIVER_NMEA_DEBUG
        printf("[--GST] invalid number of tokens: %zu\n", tokens.size());
#endif
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
#if RECEIVER_NMEA_DEBUG
        printf("[--GST] failed to parse message\n");
#endif
        delete message;
        return nullptr;
    }
}

}  // namespace nmea
}  // namespace receiver
