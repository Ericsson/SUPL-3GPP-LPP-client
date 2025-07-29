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

    DF488 prn;
    DF489 week_number;
    DF490 ura_index;
    DF491 idot;
    DF492 aode;
    DF493 toc;
    DF494 af2;
    DF495 af1;
    DF496 af0;
    DF497 aodc;
    DF498 crs;
    DF499 delta_n;
    DF500 m0;
    DF501 cuc;
    DF502 e;
    DF503 cus;
    DF504 sqrt_a;
    DF505 toe;
    DF506 cic;
    DF507 omega0;
    DF508 cis;
    DF509 i0;
    DF510 crc;
    DF511 omega;
    DF512 omega_dot;
    DF513 tgd1;
    DF514 tgd2;
    DF515 sv_health;

private:
    EXPLICIT Rtcm1042Message(std::vector<uint8_t> data) NOEXCEPT;
};

}  // namespace rtcm
}  // namespace format
