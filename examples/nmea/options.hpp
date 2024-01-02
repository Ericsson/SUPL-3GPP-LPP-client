#pragma once
#include <memory>

namespace receiver {
namespace nmea {
class NmeaReceiver;
}  // namespace nmea
}  // namespace receiver

extern std::unique_ptr<receiver::nmea::NmeaReceiver> parse_configuration(int argc, char** argv);
