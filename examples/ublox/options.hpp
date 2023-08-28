#pragma once
#include <memory>

namespace receiver {
namespace ublox {
class UbloxReceiver;
}  // namespace ublox
}  // namespace receiver

extern std::unique_ptr<receiver::ublox::UbloxReceiver> parse_configuration(int argc, char** argv);
