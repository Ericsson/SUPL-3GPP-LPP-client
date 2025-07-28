#pragma once
#include <format/rtcm/message.hpp>

#include <cmath>
#include <memory>

#include <time/tai.hpp>
#include "datafields.hpp"

// TODO: Put headers for messages in /messages folder

namespace format {
namespace rtcm{

class Rtcm1042Message final : public Message {
public:
    ~Rtcm1042Message() override = default;

    Rtcm1042Message(Rtcm1042Message const& other)
        : Message(other) {}
    Rtcm1042Message(Rtcm1042Message&&)                 = delete;
    Rtcm1042Message& operator=(Rtcm1042Message const&) = delete;
    Rtcm1042Message& operator=(Rtcm1042Message&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(std::vector<uint8_t> data);

private:
    EXPLICIT Rtcm1042Message(std::vector<uint8_t> data) NOEXCEPT;
};

}  // namespace rtcm
}  // namespace format
