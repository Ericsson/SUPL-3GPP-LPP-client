#pragma once
#include <format/rtcm/message.hpp>

#include <cmath>
#include <memory>

#include <time/tai.hpp>
#include "datafields.hpp"

namespace format {
namespace rtcm {

class Rtcm1046 final : public Message {
public:
    EXPLICIT Rtcm1046(DF002 type, std::vector<uint8_t> data) NOEXCEPT;
    ~Rtcm1046() override = default;

    Rtcm1046(Rtcm1046 const& other) : Message(other) {}
    Rtcm1046(Rtcm1046&&)                 = delete;
    Rtcm1046& operator=(Rtcm1046 const&) = delete;
    Rtcm1046& operator=(Rtcm1046&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(std::vector<uint8_t> data);

    DF252 prn;
    DF289 week_number;
    DF290 iod_nav;
    DF286 sisa_index;
    DF292 idot;
    DF293 toc;
    DF294 af2;
    DF295 af1;
    DF296 af0;
    DF297 crs;
    DF298 delta_n;
    DF299 m0;
    DF300 cuc;
    DF301 e;
    DF302 cus;
    DF303 sqrt_a;
    DF304 toe;
    DF305 cic;
    DF306 omega0;
    DF307 cis;
    DF308 i0;
    DF309 crc;
    DF310 omega;
    DF311 omega_dot;
    DF312 BGD_E5a_E1;
    DF313 BGD_E5b_E1;
    DF316 E5b_signal_health;
    DF317 E5b_data_validity;
    DF287 E1_B_signal_health;
    DF288 E1_B_data_validity;
    DF001 reserved;

private:
    EXPLICIT Rtcm1046(std::vector<uint8_t> data) NOEXCEPT;
};

}  // namespace rtcm
}  // namespace format
