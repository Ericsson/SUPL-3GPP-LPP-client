#include "1019.hpp"

#include <bitset>
#include <loglet/loglet.hpp>
#include <helper.hpp>
#include <datafields.hpp>
#include <iostream>

LOGLET_MODULE3(format, rtcm, rtcm1019);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, rtcm, rtcm1019)

namespace format {
namespace rtcm {

Rtcm1019Message::Rtcm1019Message(std::vector<uint8_t> mData) NOEXCEPT
    : Message{mData} {}

void Rtcm1019Message::print() const NOEXCEPT {
    std::cout << "RTCM 1019 message\n"
              << prn
              << week
              << SV_ACCURACY
              << code_on_l2
              << idot
              << iode
              << t_oc
              << a_f2
              << a_f1
              << a_f0
              << iodc
              << C_rs
              << delta_n
              << M_0
              << C_uc
              << e
              << C_us
              << sqrt_A
              << t_oe
              << C_ic
              << OMEGA_0
              << C_is
              << i_0
              << C_rc
              << omega
              << OMEGADOT
              << t_GD
              << SV_HEALTH
              << L2_P_data_flag
              << fit
              << std::endl;
}

std::unique_ptr<Message> Rtcm1019Message::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new Rtcm1019Message(*this));
}

std::unique_ptr<Message> Rtcm1019Message::parse(std::vector<uint8_t> mData) {
    if (mData.size()*8 < 8+16+488+24) {
        ERRORF("RTCM 1019 message created without enough data (requires %d bits, received %d bits)", 8+16+488+24, mData.size()*8);
        return std::make_unique<ErrorMessage>();
    }

    auto m = new Rtcm1019Message(mData);
    std::bitset<8+16+488+24> bits { 0UL };
    for (auto b : mData) {
        const std::bitset<8+16+488+24> bs {b};
        bits <<= 8;
        bits  |= bs;
    }

    std::size_t i = 8+16; 
    getdatafield(bits,i,  m->mType);
    if (m->mType != 1019) {
        ERRORF("RTCM 1019 message missmatched message number. should be '1019', was '%4d'", m->mType);
        return std::make_unique<ErrorMessage>();
    }
    getdatafield(bits,i,  m->prn           );
    getdatafield(bits,i,  m->week          );
    getdatafield(bits,i,  m->SV_ACCURACY   );
    getdatafield(bits,i,  m->code_on_l2    );
    getdatafield(bits,i,  m->idot          );
    getdatafield(bits,i,  m->iode          );
    getdatafield(bits,i,  m->t_oc          );
    getdatafield(bits,i,  m->a_f2          );
    getdatafield(bits,i,  m->a_f1          );
    getdatafield(bits,i,  m->a_f0          );
    getdatafield(bits,i,  m->iodc          );
    getdatafield(bits,i,  m->C_rs          );
    getdatafield(bits,i,  m->delta_n       );
    getdatafield(bits,i,  m->M_0           );
    getdatafield(bits,i,  m->C_uc          );
    getdatafield(bits,i,  m->e             );
    getdatafield(bits,i,  m->C_us          );
    getdatafield(bits,i,  m->sqrt_A        );
    getdatafield(bits,i,  m->t_oe          );
    getdatafield(bits,i,  m->C_ic          );
    getdatafield(bits,i,  m->OMEGA_0       );
    getdatafield(bits,i,  m->C_is          );
    getdatafield(bits,i,  m->i_0           );
    getdatafield(bits,i,  m->C_rc          );
    getdatafield(bits,i,  m->omega         );
    getdatafield(bits,i,  m->OMEGADOT      );
    getdatafield(bits,i,  m->t_GD          );
    getdatafield(bits,i,  m->SV_HEALTH     );
    getdatafield(bits,i,  m->L2_P_data_flag);
    getdatafield(bits,i,  m->fit           );

    return std::unique_ptr<Rtcm1019Message>(m);
}

}  // namespace rtcm
}  // namespace format
