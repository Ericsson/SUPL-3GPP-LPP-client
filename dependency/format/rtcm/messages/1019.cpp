#include "1019.hpp"

#include <bitset>
#include <loglet/loglet.hpp>
#include <helper.hpp>
#include <datafields.hpp>

LOGLET_MODULE3(format, rtcm, rtcm1019);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, rtcm, rtcm1019)

namespace format {
namespace rtcm {

Rtcm1019Message::Rtcm1019Message(std::vector<uint8_t> mData) NOEXCEPT
    : Message{mData} {}

void Rtcm1019Message::print() const NOEXCEPT {
    printf("[%4d]\n",           static_cast<int>(mType));
    printf("        prn: %d\n", static_cast<int>(prn));
}

std::unique_ptr<Message> Rtcm1019Message::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new Rtcm1019Message(*this));
}

std::unique_ptr<Message> Rtcm1019Message::parse(std::vector<uint8_t> mData) {
    if (mData.size() != 6+12+488+24) {
        ERRORF("RTCM 1019 message created without enough data");
        return std::make_unique<ErrorMessage>();
    }

    auto m = new Rtcm1019Message(mData);
    const std::bitset<6+12+488+24> bits { mData.data() };

    std::size_t i = 6+12; 
    DF002 message_number;
    getdatafield(bits,i,  message_number);
    if (message_number != 1019) {
        ERRORF("RTCM 1019 message missmatched message number. should be '1019', was '%4d'", message_number);
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
    getdatafield(bits,i,  m->dn            );
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
