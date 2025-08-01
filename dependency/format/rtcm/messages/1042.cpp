#include "1042.hpp"

#include <bitset>
#include <loglet/loglet.hpp>
#include <helper.hpp>
#include <datafields.hpp>

LOGLET_MODULE3(format, rtcm, rtcm1042);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, rtcm, rtcm1042)

namespace format {
namespace rtcm {

Rtcm1042::Rtcm1042(std::vector<uint8_t> mData) NOEXCEPT
    : Message{mData} {}

void Rtcm1042::print() const NOEXCEPT {
}

std::unique_ptr<Message> Rtcm1042::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new Rtcm1042(*this));
}

std::unique_ptr<Message> Rtcm1042::parse(std::vector<uint8_t> mData) {
    if (mData.size()*8 < 8+16+512+24) {
        ERRORF("RTCM 1042 message created without enough data (requires %d bits, received %d bits)", 8+16+512+24, mData.size()*8);
        return std::make_unique<ErrorMessage>();
    }

    auto m = new Rtcm1042(mData);
    std::bitset<8+16+512+24> bits { 0UL };
    for (auto b : mData) {
        const std::bitset<8+16+512+24> bs {b};
        bits <<= 8;
        bits  |= bs;
    }

    std::size_t i = 8+16; 
    getdatafield(bits,i,  m->mType);
    if (m->mType != 1042) {
        ERRORF("RTCM 1042 message missmatched message number. should be '1042', was '%4d'", m->mType);
        ERRORF("bits: %s", bits.to_string().c_str());
        return std::make_unique<ErrorMessage>();
    }

    getdatafield(bits,i,  m->prn        );
    getdatafield(bits,i,  m->week_number);
    getdatafield(bits,i,  m->ura_index  );
    getdatafield(bits,i,  m->idot       );
    getdatafield(bits,i,  m->aode       );
    getdatafield(bits,i,  m->toc        );
    getdatafield(bits,i,  m->af2        );
    getdatafield(bits,i,  m->af1        );
    getdatafield(bits,i,  m->af0        );
    getdatafield(bits,i,  m->aodc       );
    getdatafield(bits,i,  m->crs        );
    getdatafield(bits,i,  m->delta_n    );
    getdatafield(bits,i,  m->m0         );
    getdatafield(bits,i,  m->cuc        );
    getdatafield(bits,i,  m->e          );
    getdatafield(bits,i,  m->cus        );
    getdatafield(bits,i,  m->sqrt_a     );
    getdatafield(bits,i,  m->toe        );
    getdatafield(bits,i,  m->cic        );
    getdatafield(bits,i,  m->omega0     );
    getdatafield(bits,i,  m->cis        );
    getdatafield(bits,i,  m->i0         );
    getdatafield(bits,i,  m->crc        );
    getdatafield(bits,i,  m->omega      );
    getdatafield(bits,i,  m->omega_dot  );
    getdatafield(bits,i,  m->tgd1       );
    getdatafield(bits,i,  m->tgd2       );
    getdatafield(bits,i,  m->sv_health  );

    return std::unique_ptr<Rtcm1042>(m);
}

}  // namespace rtcm
}  // namespace format
