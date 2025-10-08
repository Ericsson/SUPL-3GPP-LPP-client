#pragma once
#include <format/ctrl/message.hpp>

namespace format {
namespace ctrl {

class IdentityImsi : public Message {
public:
    IdentityImsi(std::string message, uint64_t imsi) NOEXCEPT;
    ~IdentityImsi() override;

    IdentityImsi(IdentityImsi const& other) : Message(other), mImsi(other.mImsi) {}
    IdentityImsi(IdentityImsi&&)                 = delete;
    IdentityImsi& operator=(IdentityImsi const&) = delete;
    IdentityImsi& operator=(IdentityImsi&&)      = delete;

    NODISCARD uint64_t imsi() const NOEXCEPT { return mImsi; }

    void      print() const NOEXCEPT override;
    NODISCARD std::unique_ptr<Message> clone() const NOEXCEPT override;

private:
    uint64_t mImsi;
};

}  // namespace ctrl
}  // namespace format
