#include <receiver/nmea/epe.hpp>
#include "helper.hpp"

namespace receiver {
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

EpeMessage::EpeMessage(std::string prefix, std::string payload, std::string checksum) NMEA_NOEXCEPT
    : Message{prefix, payload, checksum},
      mMsgVer{0.0},
      mNorth{0.0},
      mEast{0.0},
      mDown{0.0},
      m2D{0.0},
      m3D{0.0} {}

void EpeMessage::print() const NMEA_NOEXCEPT {
    printf("[%5s]\n", prefix().c_str());
    printf("  message version: %f\n", mMsgVer);
    printf("  north error: %f\n", mNorth);
    printf("  east error: %f\n", mEast);
    printf("  down error: %f\n", mDown);
    printf("  2D error: %f\n", m2D);
    printf("  3D error: %f\n", m3D);
}

std::unique_ptr<Message> EpeMessage::parse(std::string prefix, std::string const& payload,
                                           std::string checksum) {
    // split payload by ','
    auto tokens = split(payload, ',');

    // check number of tokens
    if (tokens.size() < 6) {
#if RECEIVER_NMEA_DEBUG
        printf("[--EPE] invalid number of tokens: %zu\n", tokens.size());
#endif
        return nullptr;
    }

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
        return std::unique_ptr<EpeMessage>(message);
    } else {
#if RECEIVER_NMEA_DEBUG
        printf("[--EPE] failed to parse message\n");
#endif
        delete message;
        return nullptr;
    }
}

}  // namespace nmea
}  // namespace receiver