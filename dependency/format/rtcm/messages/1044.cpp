#include "1044.hpp"
#include "bits.hpp"

#include <cstdio>
#include <cxx11_compat.hpp>
#include <loglet/loglet.hpp>

LOGLET_MODULE3(format, rtcm, rtcm1044);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, rtcm, rtcm1044)

namespace format {
namespace rtcm {

// header (24) + message (485) + crc (24)
CONSTEXPR static size_t RTCM1044_BITS = 24 + 485 + 24;
CONSTEXPR static double SC2RAD        = 3.1415926535897932;

Rtcm1044::Rtcm1044(std::vector<uint8_t> data) NOEXCEPT : Message{1044, std::move(data)} {}

void Rtcm1044::print() const NOEXCEPT {
    printf("[RTCM1044]\n");
    printf("  prn:       %u\n", static_cast<unsigned>(prn));
    printf("  week:      %u\n", static_cast<unsigned>(week));
    printf("  iode:      %u\n", static_cast<unsigned>(iode));
    printf("  iodc:      %u\n", static_cast<unsigned>(iodc));
    printf("  toc:       %.0f\n", toc);
    printf("  toe:       %.0f\n", toe);
    printf("  sqrt_a:    %.6f\n", sqrt_a);
    printf("  e:         %.12e\n", e);
    printf("  health:    %u\n", static_cast<unsigned>(sv_health));
}

std::unique_ptr<Message> Rtcm1044::clone() const NOEXCEPT {
    return parse(mData);
}

std::unique_ptr<Message> Rtcm1044::parse(std::vector<uint8_t> data) {
    if (data.size() * 8 < RTCM1044_BITS) {
        ERRORF("RTCM 1044 message without enough data (requires %zu bits, received %zu bits)",
               RTCM1044_BITS, data.size() * 8);
        return std::make_unique<ErrorMessage>(DF002(1044), std::move(data));
    }

    BitReader r{data, 24};
    if (r.u(12) != 1044) return std::make_unique<ErrorMessage>(DF002(1044), std::move(data));

    auto m         = new Rtcm1044(data);
    m->prn         = static_cast<uint8_t>(192 + r.u(4));
    m->toc         = r.u(16, 4);
    m->af2         = r.s(8, -55);
    m->af1         = r.s(16, -43);
    m->af0         = r.s(22, -31);
    m->iode        = static_cast<uint8_t>(r.u(8));
    m->crs         = r.s(16, -5);
    m->delta_n     = r.s(16, -43) * SC2RAD;
    m->m0          = r.s(32, -31) * SC2RAD;
    m->cuc         = r.s(16, -29);
    m->e           = r.u(32, -33);
    m->cus         = r.s(16, -29);
    m->sqrt_a      = r.u(32, -19);
    m->toe         = r.u(16, 4);
    m->cic         = r.s(16, -29);
    m->omega0      = r.s(32, -31) * SC2RAD;
    m->cis         = r.s(16, -29);
    m->i0          = r.s(32, -31) * SC2RAD;
    m->crc         = r.s(16, -5);
    m->omega       = r.s(32, -31) * SC2RAD;
    m->omega_dot   = r.s(24, -43) * SC2RAD;
    m->idot        = r.s(14, -43) * SC2RAD;
    m->codes_on_l2 = static_cast<uint8_t>(r.u(2));
    m->week        = static_cast<uint16_t>(r.u(10));
    m->ura         = static_cast<uint8_t>(r.u(4));
    m->sv_health   = static_cast<uint8_t>(r.u(6));
    m->tgd         = r.s(8, -31);
    m->iodc        = static_cast<uint16_t>(r.u(10));
    m->fit         = r.u(1) != 0;
    return std::unique_ptr<Rtcm1044>(m);
}

}  // namespace rtcm
}  // namespace format
