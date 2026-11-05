#include "1020.hpp"
#include "bits.hpp"

#include <cstdio>
#include <cxx11_compat.hpp>
#include <loglet/loglet.hpp>

LOGLET_MODULE3(format, rtcm, rtcm1020);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, rtcm, rtcm1020)

namespace format {
namespace rtcm {

// header (24) + message (360) + crc (24)
CONSTEXPR static size_t RTCM1020_BITS = 24 + 360 + 24;

Rtcm1020::Rtcm1020(std::vector<uint8_t> data) NOEXCEPT : Message{1020, std::move(data)} {}

void Rtcm1020::print() const NOEXCEPT {
    printf("[RTCM1020]\n");
    printf("  slot:      %u\n", static_cast<unsigned>(slot));
    printf("  channel:   %d\n", static_cast<int>(frequency_channel));
    printf("  tb:        %u\n", static_cast<unsigned>(tb));
    printf("  bn:        %u\n", bn_msb ? 1u : 0u);
    printf("  x:         %+.6f %+.9f %+.12e\n", position[0], velocity[0], acceleration[0]);
    printf("  y:         %+.6f %+.9f %+.12e\n", position[1], velocity[1], acceleration[1]);
    printf("  z:         %+.6f %+.9f %+.12e\n", position[2], velocity[2], acceleration[2]);
    printf("  gamma_n:   %+.12e\n", gamma_n);
    printf("  tau_n:     %+.12e\n", tau_n);
    printf("  e_n:       %u\n", static_cast<unsigned>(e_n));
    printf("  n_t:       %u\n", static_cast<unsigned>(n_t));
    printf("  n_4:       %u\n", static_cast<unsigned>(n_4));
}

std::unique_ptr<Message> Rtcm1020::clone() const NOEXCEPT {
    return parse(mData);
}

std::unique_ptr<Message> Rtcm1020::parse(std::vector<uint8_t> data) {
    if (data.size() * 8 < RTCM1020_BITS) {
        ERRORF("RTCM 1020 message without enough data (requires %zu bits, received %zu bits)",
               RTCM1020_BITS, data.size() * 8);
        return std::make_unique<ErrorMessage>(DF002(1020), std::move(data));
    }

    BitReader r{data, 24};
    if (r.u(12) != 1020) return std::make_unique<ErrorMessage>(DF002(1020), std::move(data));

    auto m                      = new Rtcm1020(data);
    m->slot                     = static_cast<uint8_t>(r.u(6));
    m->frequency_channel        = static_cast<int8_t>(static_cast<int>(r.u(5)) - 7);
    m->almanac_health           = r.u(1) != 0;
    m->almanac_health_available = r.u(1) != 0;
    m->p1                       = static_cast<uint8_t>(r.u(2));
    m->tk_h                     = static_cast<uint8_t>(r.u(5));
    m->tk_m                     = static_cast<uint8_t>(r.u(6));
    m->tk_s                     = static_cast<uint8_t>(r.u(1) * 30);
    m->bn_msb                   = r.u(1) != 0;
    m->p2                       = r.u(1) != 0;
    m->tb                       = static_cast<uint8_t>(r.u(7));
    for (int i = 0; i < 3; i++) {
        m->velocity[i]     = r.sm(24, -20);
        m->position[i]     = r.sm(27, -11);
        m->acceleration[i] = r.sm(5, -30);
    }
    m->p3              = r.u(1) != 0;
    m->gamma_n         = r.sm(11, -40);
    m->p               = static_cast<uint8_t>(r.u(2));
    m->ln3             = r.u(1) != 0;
    m->tau_n           = r.sm(22, -30);
    m->delta_tau_n     = r.sm(5, -30);
    m->e_n             = static_cast<uint8_t>(r.u(5));
    m->p4              = r.u(1) != 0;
    m->f_t             = static_cast<uint8_t>(r.u(4));
    m->n_t             = static_cast<uint16_t>(r.u(11));
    m->m               = static_cast<uint8_t>(r.u(2));
    m->additional_data = r.u(1) != 0;
    m->n_a             = static_cast<uint16_t>(r.u(11));
    m->tau_c           = r.sm(32, -31);
    m->n_4             = static_cast<uint8_t>(r.u(5));
    m->tau_gps         = r.sm(22, -30);
    m->ln5             = r.u(1) != 0;
    return std::unique_ptr<Rtcm1020>(m);
}

}  // namespace rtcm
}  // namespace format
