#include "identity.hpp"

#include <stdio.h>

namespace format {
namespace ctrl {

IdentityImsi::IdentityImsi(std::string message, uint64_t imsi) NOEXCEPT
    : Message(std::move(message)),
      mImsi(imsi) {}

IdentityImsi::~IdentityImsi() NOEXCEPT = default;

void IdentityImsi::print() const NOEXCEPT {
    printf("[ IMSI]: %" PRIu64, mImsi);
}

std::unique_ptr<Message> IdentityImsi::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new IdentityImsi(*this));
}

}  // namespace ctrl
}  // namespace format
