#pragma once
#include <format/rtcm/message.hpp>

#include <memory>

namespace format {
namespace rtcm {

/// QZSS ephemeris (RTCM 10403.4 Table 3.5-123), angles in radians
class Rtcm1044 final : public Message {
public:
    EXPLICIT Rtcm1044(std::vector<uint8_t> data) NOEXCEPT;
    ~Rtcm1044() override = default;

    Rtcm1044(Rtcm1044 const& other)      = delete;
    Rtcm1044(Rtcm1044&&)                 = delete;
    Rtcm1044& operator=(Rtcm1044 const&) = delete;
    Rtcm1044& operator=(Rtcm1044&&)      = delete;

    void      print() const NOEXCEPT override;
    NODISCARD std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(std::vector<uint8_t> data);

    uint8_t  prn;  // 193..202
    double   toc;
    double   af2;
    double   af1;
    double   af0;
    uint8_t  iode;
    double   crs;
    double   delta_n;
    double   m0;
    double   cuc;
    double   e;
    double   cus;
    double   sqrt_a;
    double   toe;
    double   cic;
    double   omega0;
    double   cis;
    double   i0;
    double   crc;
    double   omega;
    double   omega_dot;
    double   idot;
    uint8_t  codes_on_l2;
    uint16_t week;
    uint8_t  ura;
    uint8_t  sv_health;
    double   tgd;
    uint16_t iodc;
    bool     fit;
};

}  // namespace rtcm
}  // namespace format
