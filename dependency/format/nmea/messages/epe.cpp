#include "epe.hpp"
#include "helper.hpp"

#include <cmath>
#include <loglet/loglet.hpp>

LOGLET_MODULE3(format, nmea, epe);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, nmea, epe)

namespace format {
namespace nmea {

double EpeMessage::semi_major() const NOEXCEPT {
    return m2D / sqrt(2);
}

double EpeMessage::semi_minor() const NOEXCEPT {
    return m2D / sqrt(2);
}

double EpeMessage::vertical_position_error() const NOEXCEPT {
    return sqrt((m3D * m3D) - (m2D * m2D));
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

EpeMessage::EpeMessage(std::string prefix, std::string payload, std::string checksum) NOEXCEPT
    : Message{prefix, payload, checksum},
      mMsgVer{0.0},
      mNorth{0.0},
      mEast{0.0},
      mDown{0.0},
      m2D{0.0},
      m3D{0.0} {}

void EpeMessage::print() const NOEXCEPT {
    printf("[%5s]\n", prefix().c_str());
    printf("  message version: %f\n", mMsgVer);
    printf("  north error: %f\n", mNorth);
    printf("  east error: %f\n", mEast);
    printf("  down error: %f\n", mDown);
    printf("  2D error: %f\n", m2D);
    printf("  3D error: %f\n", m3D);
}

std::unique_ptr<Message> EpeMessage::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new EpeMessage(*this));
}

std::unique_ptr<Message> EpeMessage::parse(std::string prefix, std::string const& payload,
                                           std::string checksum) {
    FUNCTION_SCOPEF("%s,%s*%s", prefix.c_str(), payload.c_str(), checksum.c_str());
    auto tokens = split(payload, ',');

    if (tokens.size() < 6) {
        VERBOSEF("invalid token count: %zu (expected >= 6)", tokens.size());
        return nullptr;
    }

    VERBOSEF("token count: %zu", tokens.size());

    // parse
    auto message = new EpeMessage(prefix, payload, checksum);
    auto success = true;
    success &= parse_double_opt(tokens[0], message->mMsgVer);
    success &= parse_double_opt(tokens[1], message->mNorth);
    success &= parse_double_opt(tokens[2], message->mEast);
    success &= parse_double_opt(tokens[3], message->mDown);
    success &= parse_double_opt(tokens[4], message->m2D);
    success &= parse_double_opt(tokens[5], message->m3D);

    if (success) {
        TRACEF("EPE message parsed successfully");
        return std::unique_ptr<EpeMessage>(message);
    } else {
        VERBOSEF("failed to parse EPE message");
        delete message;
        return nullptr;
    }
}

}  // namespace nmea
}  // namespace format
