#include "1046.hpp"

#include <bitset>
#include <cxx11_compat.hpp>
#include <datafields.hpp>
#include <helper.hpp>
#include <loglet/loglet.hpp>
#include <stdio.h>

LOGLET_MODULE3(format, rtcm, rtcm1046);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, rtcm, rtcm1046)

namespace format {
namespace rtcm {

Rtcm1046::Rtcm1046(DF002 type, std::vector<uint8_t> data) NOEXCEPT
    : Message{type, std::move(data)} {}

void Rtcm1046::print() const NOEXCEPT {
    printf("[RTCM1046]\n");
    printf("  prn:                %u\n", static_cast<unsigned>(prn.value()));
    printf("  week_number:        %u\n", static_cast<unsigned>(week_number.value()));
    printf("  iod_nav:            %u\n", static_cast<unsigned>(iod_nav.value()));
    printf("  sisa_index:         %u\n", static_cast<unsigned>(sisa_index.value()));
    printf("  idot:               %.12e\n", idot.value());
    printf("  toc:                %.6f\n", toc.value());
    printf("  af2:                %.12e\n", af2.value());
    printf("  af1:                %.12e\n", af1.value());
    printf("  af0:                %.12e\n", af0.value());
    printf("  crs:                %.6f\n", crs.value());
    printf("  delta_n:            %.12e\n", delta_n.value());
    printf("  m0:                 %.12e\n", m0.value());
    printf("  cuc:                %.12e\n", cuc.value());
    printf("  e:                  %.12e\n", e.value());
    printf("  cus:                %.12e\n", cus.value());
    printf("  sqrt_a:             %.6f\n", sqrt_a.value());
    printf("  toe:                %.6f\n", toe.value());
    printf("  cic:                %.12e\n", cic.value());
    printf("  omega0:             %.12e\n", omega0.value());
    printf("  cis:                %.12e\n", cis.value());
    printf("  i0:                 %.12e\n", i0.value());
    printf("  crc:                %.6f\n", crc.value());
    printf("  omega:              %.12e\n", omega.value());
    printf("  omega_dot:          %.12e\n", omega_dot.value());
    printf("  BGD_E5a_E1:         %.12e\n", BGD_E5a_E1.value());
    printf("  BGD_E5b_E1:         %.12e\n", BGD_E5b_E1.value());
    printf("  E5b_signal_health:  %u\n", static_cast<unsigned>(E5b_signal_health.value()));
    printf("  E5b_data_validity:  %s\n", E5b_data_validity.value() ? "true" : "false");
    printf("  E1_B_signal_health: %u\n", static_cast<unsigned>(E1_B_signal_health.value()));
    printf("  E1_B_data_validity: %s\n", E1_B_data_validity.value() ? "true" : "false");
    printf("  reserved:           %u\n", static_cast<unsigned>(reserved.value()));
}

std::unique_ptr<Message> Rtcm1046::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new Rtcm1046(*this));
}

std::unique_ptr<Message> Rtcm1046::parse(std::vector<uint8_t> data) {
    if (data.size() * 8 < 8 + 16 + 504 + 24) {
        ERRORF("RTCM 1046 message without enough data (requires %d bits, received %d bits)",
               8 + 16 + 504 + 24, data.size() * 8);
        return std::make_unique<ErrorMessage>(1046, std::move(data));
    }

    if (data.size() * 8 > 8 + 16 + 504 + 24) {
        WARNF("RTCM 1046 message with too much data (requires %d bits, received %d bits)",
              8 + 16 + 504 + 24, data.size() * 8);
    }

    std::bitset<8 + 16 + 504 + 24> bits;
    for (std::size_t i = 0; i < (8 + 16 + 504 + 24) / 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            bits[bits.size() - 1 - (i * 8 + j)] = (data[i] >> (7 - j)) & 1;
        }
    }

    DF002       type;
    std::size_t i = 8 + 16;
    getdatafield(bits, i, type);
    if (type != 1046) {
        ERRORF("RTCM 1046 message missmatched message number. should be '1046', was '%4d'", type);
        return std::make_unique<ErrorMessage>(1046, std::move(data));
    }

    auto m   = new Rtcm1046(1046, data);
    m->mType = DF002(1046);
    getdatafield(bits, i, m->prn);
    getdatafield(bits, i, m->week_number);
    getdatafield(bits, i, m->iod_nav);
    getdatafield(bits, i, m->sisa_index);
    getdatafield(bits, i, m->idot);
    getdatafield(bits, i, m->toc);
    getdatafield(bits, i, m->af2);
    getdatafield(bits, i, m->af1);
    getdatafield(bits, i, m->af0);
    getdatafield(bits, i, m->crs);
    getdatafield(bits, i, m->delta_n);
    getdatafield(bits, i, m->m0);
    getdatafield(bits, i, m->cuc);
    getdatafield(bits, i, m->e);
    getdatafield(bits, i, m->cus);
    getdatafield(bits, i, m->sqrt_a);
    getdatafield(bits, i, m->toe);
    getdatafield(bits, i, m->cic);
    getdatafield(bits, i, m->omega0);
    getdatafield(bits, i, m->cis);
    getdatafield(bits, i, m->i0);
    getdatafield(bits, i, m->crc);
    getdatafield(bits, i, m->omega);
    getdatafield(bits, i, m->omega_dot);
    getdatafield(bits, i, m->BGD_E5a_E1);
    getdatafield(bits, i, m->BGD_E5b_E1);
    getdatafield(bits, i, m->E5b_signal_health);
    getdatafield(bits, i, m->E5b_data_validity);
    getdatafield(bits, i, m->E1_B_signal_health);
    getdatafield(bits, i, m->E1_B_data_validity);
    getdatafield(bits, i, m->reserved);

    return std::unique_ptr<Rtcm1046>(m);
}

}  // namespace rtcm
}  // namespace format
