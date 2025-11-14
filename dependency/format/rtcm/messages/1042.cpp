#include "1042.hpp"

#include <bitset>
#include <cstdio>
#include <cxx11_compat.hpp>
#include <datafields.hpp>
#include <helper.hpp>
#include <loglet/loglet.hpp>

LOGLET_MODULE3(format, rtcm, rtcm1042);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, rtcm, rtcm1042)

namespace format {
namespace rtcm {

Rtcm1042::Rtcm1042(DF002 type, std::vector<uint8_t> data) NOEXCEPT
    : Message{type, std::move(data)} {}

void Rtcm1042::print() const NOEXCEPT {
    printf("[RTCM1042]\n");
    printf("  prn:         %u\n", static_cast<unsigned>(prn.value()));
    printf("  week_number: %u\n", static_cast<unsigned>(week_number.value()));
    printf("  ura_index:   %u\n", static_cast<unsigned>(ura_index.value()));
    printf("  idot:        %.12e\n", idot.value());
    printf("  aode:        %u\n", static_cast<unsigned>(aode.value()));
    printf("  toc:         %.6f\n", toc.value());
    printf("  af2:         %.12e\n", af2.value());
    printf("  af1:         %.12e\n", af1.value());
    printf("  af0:         %.12e\n", af0.value());
    printf("  aodc:        %u\n", static_cast<unsigned>(aodc.value()));
    printf("  crs:         %.6f\n", crs.value());
    printf("  delta_n:     %.12e\n", delta_n.value());
    printf("  m0:          %.12e\n", m0.value());
    printf("  cuc:         %.12e\n", cuc.value());
    printf("  e:           %.12e\n", e.value());
    printf("  cus:         %.12e\n", cus.value());
    printf("  sqrt_a:      %.6f\n", sqrt_a.value());
    printf("  toe:         %.6f\n", toe.value());
    printf("  cic:         %.12e\n", cic.value());
    printf("  omega0:      %.12e\n", omega0.value());
    printf("  cis:         %.12e\n", cis.value());
    printf("  i0:          %.12e\n", i0.value());
    printf("  crc:         %.6f\n", crc.value());
    printf("  omega:       %.12e\n", omega.value());
    printf("  omega_dot:   %.12e\n", omega_dot.value());
    printf("  tgd1:        %.12e\n", tgd1.value());
    printf("  tgd2:        %.12e\n", tgd2.value());
    printf("  sv_health:   %u\n", static_cast<unsigned>(sv_health.value()));
}

std::unique_ptr<Message> Rtcm1042::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new Rtcm1042(*this));
}

std::unique_ptr<Message> Rtcm1042::parse(std::vector<uint8_t> data) {
    if (data.size() * 8 < 8 + 16 + 512 + 24) {
        ERRORF("RTCM 1042 message without enough data (requires %d bits, received %d bits)",
               8 + 16 + 512 + 24, data.size() * 8);
        return std::make_unique<ErrorMessage>(1042, std::move(data));
    }

    if (data.size() * 8 > 8 + 16 + 512 + 24) {
        WARNF("RTCM 1042 message with too much data (requires %d bits, received %d bits)",
              8 + 16 + 512 + 24, data.size() * 8);
    }

    std::bitset<8 + 16 + 512 + 24> bits;
    for (std::size_t i = 0; i < (8 + 16 + 512 + 24) / 8; ++i) {
        for (std::size_t j = 0; j < 8; ++j) {
            bits[bits.size() - 1 - (i * 8 + j)] = (data[i] >> (7 - j)) & 1;
        }
    }

    DF002       type;
    std::size_t i = 8 + 16;
    getdatafield(bits, i, type);
    if (type != 1042) {
        ERRORF("RTCM 1042 message missmatched message number. should be '1042', was '%4d'",
               type.value());
        return std::make_unique<ErrorMessage>(1042, std::move(data));
    }

    auto m   = new Rtcm1042(1042, data);
    m->mType = DF002(1042);

    getdatafield(bits, i, m->prn);
    getdatafield(bits, i, m->week_number);
    getdatafield(bits, i, m->ura_index);
    getdatafield(bits, i, m->idot);
    getdatafield(bits, i, m->aode);
    getdatafield(bits, i, m->toc);
    getdatafield(bits, i, m->af2);
    getdatafield(bits, i, m->af1);
    getdatafield(bits, i, m->af0);
    getdatafield(bits, i, m->aodc);
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
    getdatafield(bits, i, m->tgd1);
    getdatafield(bits, i, m->tgd2);
    getdatafield(bits, i, m->sv_health);

    return std::unique_ptr<Rtcm1042>(m);
}

}  // namespace rtcm
}  // namespace format
