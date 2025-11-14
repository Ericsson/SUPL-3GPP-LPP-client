#include "1019.hpp"

#include <bitset>
#include <cstdio>
#include <cxx11_compat.hpp>
#include <datafields.hpp>
#include <helper.hpp>
#include <loglet/loglet.hpp>

LOGLET_MODULE3(format, rtcm, rtcm1019);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, rtcm, rtcm1019)

namespace format {
namespace rtcm {

Rtcm1019::Rtcm1019(DF002 type, std::vector<uint8_t> data) NOEXCEPT
    : Message{type, std::move(data)} {}

void Rtcm1019::print() const NOEXCEPT {
    printf("[RTCM1019]\n");
    printf("  prn:         %u\n", static_cast<unsigned>(prn.value()));
    printf("  week:        %u\n", static_cast<unsigned>(week.value()));
    printf("  sv_accuracy: %u\n", static_cast<unsigned>(sv_accuracy.value()));
    printf("  code_on_l2:  %u\n", static_cast<unsigned>(code_on_l2.value()));
    printf("  idot:        %.12e\n", idot.value());
    printf("  iode:        %u\n", static_cast<unsigned>(iode.value()));
    printf("  t_oc:        %.6f\n", t_oc.value());
    printf("  a_f2:        %.12e\n", a_f2.value());
    printf("  a_f1:        %.12e\n", a_f1.value());
    printf("  a_f0:        %.12e\n", a_f0.value());
    printf("  iodc:        %u\n", static_cast<unsigned>(iodc.value()));
    printf("  c_rs:        %.6f\n", c_rs.value());
    printf("  delta_n:     %.12e\n", delta_n.value());
    printf("  m_0:         %.12e\n", m_0.value());
    printf("  c_uc:        %.12e\n", c_uc.value());
    printf("  e:           %.12e\n", e.value());
    printf("  c_us:        %.12e\n", c_us.value());
    printf("  sqrt_a:      %.6f\n", sqrt_a.value());
    printf("  t_oe:        %.6f\n", t_oe.value());
    printf("  c_ic:        %.12e\n", c_ic.value());
    printf("  omega_0:     %.12e\n", omega_0.value());
    printf("  c_is:        %.12e\n", c_is.value());
    printf("  i_0:         %.12e\n", i_0.value());
    printf("  c_rc:        %.6f\n", c_rc.value());
    printf("  omega:       %.12e\n", omega.value());
    printf("  omegadot:    %.12e\n", omegadot.value());
    printf("  t_gd:        %.12e\n", t_gd.value());
    printf("  sv_health:   %u\n", static_cast<unsigned>(sv_health.value()));
    printf("  L2_P_flag:   %s\n", l2_p_data_flag.value() ? "true" : "false");
    printf("  fit:         %s\n", fit.value() ? "true" : "false");
}

std::unique_ptr<Message> Rtcm1019::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new Rtcm1019(*this));
}

std::unique_ptr<Message> Rtcm1019::parse(std::vector<uint8_t> data) {
    if (data.size() * 8 < 8 + 16 + 488 + 24) {
        ERRORF("RTCM 1019 message without enough data (requires %d bits, received %d bits)",
               8 + 16 + 488 + 24, data.size() * 8);
        return std::make_unique<ErrorMessage>(DF002(1019), std::move(data));
    }

    if (data.size() * 8 > 8 + 16 + 488 + 24) {
        WARNF("RTCM 1019 message with too much data (requires %d bits, received %d bits)",
              8 + 16 + 488 + 24, data.size() * 8);
    }

    std::bitset<8 + 16 + 488 + 24> bits;
    for (std::size_t i = 0; i < (8 + 16 + 488 + 24) / 8; ++i) {
        for (std::size_t j = 0; j < 8; ++j) {
            bits[bits.size() - 1 - (i * 8 + j)] = (data[i] >> (7 - j)) & 1;
        }
    }

    DF002       type;
    std::size_t i = 8 + 16;
    getdatafield(bits, i, type);
    if (type != 1019) {
        ERRORF("RTCM 1019 message missmatched message number. should be '1019', was '%4d'",
               type.value());
        return std::make_unique<ErrorMessage>(DF002(1019), std::move(data));
    }

    auto m   = new Rtcm1019(1019, data);
    m->mType = DF002(1019);
    getdatafield(bits, i, m->prn);
    getdatafield(bits, i, m->week);
    getdatafield(bits, i, m->sv_accuracy);
    getdatafield(bits, i, m->code_on_l2);
    getdatafield(bits, i, m->idot);
    getdatafield(bits, i, m->iode);
    getdatafield(bits, i, m->t_oc);
    getdatafield(bits, i, m->a_f2);
    getdatafield(bits, i, m->a_f1);
    getdatafield(bits, i, m->a_f0);
    getdatafield(bits, i, m->iodc);
    getdatafield(bits, i, m->c_rs);
    getdatafield(bits, i, m->delta_n);
    getdatafield(bits, i, m->m_0);
    getdatafield(bits, i, m->c_uc);
    getdatafield(bits, i, m->e);
    getdatafield(bits, i, m->c_us);
    getdatafield(bits, i, m->sqrt_a);
    getdatafield(bits, i, m->t_oe);
    getdatafield(bits, i, m->c_ic);
    getdatafield(bits, i, m->omega_0);
    getdatafield(bits, i, m->c_is);
    getdatafield(bits, i, m->i_0);
    getdatafield(bits, i, m->c_rc);
    getdatafield(bits, i, m->omega);
    getdatafield(bits, i, m->omegadot);
    getdatafield(bits, i, m->t_gd);
    getdatafield(bits, i, m->sv_health);
    getdatafield(bits, i, m->l2_p_data_flag);
    getdatafield(bits, i, m->fit);

    return std::unique_ptr<Rtcm1019>(m);
}

}  // namespace rtcm
}  // namespace format
