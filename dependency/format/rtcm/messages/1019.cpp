#include "1019.hpp"

#include <bitset>
#include <cxx11_compat.hpp>
#include <datafields.hpp>
#include <helper.hpp>
#include <loglet/loglet.hpp>
#include <stdio.h>

LOGLET_MODULE3(format, rtcm, rtcm1019);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, rtcm, rtcm1019)

namespace format {
namespace rtcm {

Rtcm1019::Rtcm1019(DF002 type, std::vector<uint8_t> data) NOEXCEPT
    : Message{type, std::move(data)} {}

void Rtcm1019::print() const NOEXCEPT {
    printf("[RTCM1019]\n");
    printf("  prn:         %u\n", static_cast<unsigned>(prn.value()));
    printf("  week:        %u\n", static_cast<unsigned>(week.value()));
    printf("  sv_accuracy: %u\n", static_cast<unsigned>(SV_ACCURACY.value()));
    printf("  code_on_l2:  %u\n", static_cast<unsigned>(code_on_l2.value()));
    printf("  idot:        %.12e\n", idot.value());
    printf("  iode:        %u\n", static_cast<unsigned>(iode.value()));
    printf("  t_oc:        %.6f\n", t_oc.value());
    printf("  a_f2:        %.12e\n", a_f2.value());
    printf("  a_f1:        %.12e\n", a_f1.value());
    printf("  a_f0:        %.12e\n", a_f0.value());
    printf("  iodc:        %u\n", static_cast<unsigned>(iodc.value()));
    printf("  C_rs:        %.6f\n", C_rs.value());
    printf("  delta_n:     %.12e\n", delta_n.value());
    printf("  M_0:         %.12e\n", M_0.value());
    printf("  C_uc:        %.12e\n", C_uc.value());
    printf("  e:           %.12e\n", e.value());
    printf("  C_us:        %.12e\n", C_us.value());
    printf("  sqrt_A:      %.6f\n", sqrt_A.value());
    printf("  t_oe:        %.6f\n", t_oe.value());
    printf("  C_ic:        %.12e\n", C_ic.value());
    printf("  OMEGA_0:     %.12e\n", OMEGA_0.value());
    printf("  C_is:        %.12e\n", C_is.value());
    printf("  i_0:         %.12e\n", i_0.value());
    printf("  C_rc:        %.6f\n", C_rc.value());
    printf("  omega:       %.12e\n", omega.value());
    printf("  OMEGADOT:    %.12e\n", OMEGADOT.value());
    printf("  t_GD:        %.12e\n", t_GD.value());
    printf("  SV_HEALTH:   %u\n", static_cast<unsigned>(SV_HEALTH.value()));
    printf("  L2_P_flag:   %s\n", L2_P_data_flag.value() ? "true" : "false");
    printf("  fit:         %s\n", fit.value() ? "true" : "false");
}

std::unique_ptr<Message> Rtcm1019::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new Rtcm1019(*this));
}

std::unique_ptr<Message> Rtcm1019::parse(std::vector<uint8_t> data) {
    if (data.size() * 8 < 8 + 16 + 488 + 24) {
        ERRORF("RTCM 1019 message without enough data (requires %d bits, received %d bits)",
               8 + 16 + 488 + 24, data.size() * 8);
        return std::make_unique<ErrorMessage>(1019, std::move(data));
    }

    if (data.size() * 8 > 8 + 16 + 488 + 24) {
        WARNF("RTCM 1019 message with too much data (requires %d bits, received %d bits)",
              8 + 16 + 488 + 24, data.size() * 8);
    }

    std::bitset<8 + 16 + 488 + 24> bits;
    for (std::size_t i = 0; i < (8 + 16 + 488 + 24) / 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            bits[bits.size() - 1 - (i * 8 + j)] = (data[i] >> (7 - j)) & 1;
        }
    }

    DF002       type;
    std::size_t i = 8 + 16;
    getdatafield(bits, i, type);
    if (type != 1019) {
        ERRORF("RTCM 1019 message missmatched message number. should be '1019', was '%4d'", type);
        return std::make_unique<ErrorMessage>(1019, std::move(data));
    }

    auto m   = new Rtcm1019(1019, data);
    m->mType = DF002(1019);
    getdatafield(bits, i, m->prn);
    getdatafield(bits, i, m->week);
    getdatafield(bits, i, m->SV_ACCURACY);
    getdatafield(bits, i, m->code_on_l2);
    getdatafield(bits, i, m->idot);
    getdatafield(bits, i, m->iode);
    getdatafield(bits, i, m->t_oc);
    getdatafield(bits, i, m->a_f2);
    getdatafield(bits, i, m->a_f1);
    getdatafield(bits, i, m->a_f0);
    getdatafield(bits, i, m->iodc);
    getdatafield(bits, i, m->C_rs);
    getdatafield(bits, i, m->delta_n);
    getdatafield(bits, i, m->M_0);
    getdatafield(bits, i, m->C_uc);
    getdatafield(bits, i, m->e);
    getdatafield(bits, i, m->C_us);
    getdatafield(bits, i, m->sqrt_A);
    getdatafield(bits, i, m->t_oe);
    getdatafield(bits, i, m->C_ic);
    getdatafield(bits, i, m->OMEGA_0);
    getdatafield(bits, i, m->C_is);
    getdatafield(bits, i, m->i_0);
    getdatafield(bits, i, m->C_rc);
    getdatafield(bits, i, m->omega);
    getdatafield(bits, i, m->OMEGADOT);
    getdatafield(bits, i, m->t_GD);
    getdatafield(bits, i, m->SV_HEALTH);
    getdatafield(bits, i, m->L2_P_data_flag);
    getdatafield(bits, i, m->fit);

    return std::unique_ptr<Rtcm1019>(m);
}

}  // namespace rtcm
}  // namespace format
