#include "1046.hpp"

#include <bitset>
#include <loglet/loglet.hpp>
#include <helper.hpp>
#include <datafields.hpp>

LOGLET_MODULE3(format, rtcm, rtcm1046);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, rtcm, rtcm1046)

namespace format {
namespace rtcm {

Rtcm1046::Rtcm1046(DF002 type, std::vector<uint8_t> data) NOEXCEPT
    : Message{type, std::move(data)} {}

void Rtcm1046::print() const NOEXCEPT {
    std::cout << "RTCM 1046 message\n"
              << prn               
              << week_number       
              << iod_nav           
              << sisa_index        
              << idot              
              << toc               
              << af2               
              << af1               
              << af0               
              << crs               
              << delta_n           
              << m0                
              << cuc               
              << e                 
              << cus               
              << sqrt_a            
              << toe               
              << cic               
              << omega0           
              << cis               
              << i0                
              << crc               
              << omega             
              << omega_dot         
              << BGD_E5a_E1        
              << BGD_E5b_E1        
              << E5b_signal_health 
              << E5b_data_validity 
              << E1_B_signal_health
              << E1_B_data_validity
              << reserved
              << std::endl;
}

std::unique_ptr<Message> Rtcm1046::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new Rtcm1046(*this));
}

std::unique_ptr<Message> Rtcm1046::parse(std::vector<uint8_t> data) {
    if (data.size()*8 < 8+16+504+24) {
        ERRORF("RTCM 1046 message created without enough data (requires %d bits, received %d bits)", 8+16+504+24, data.size()*8);
        return std::make_unique<ErrorMessage>(1046, std::move(data));
    }

    auto m = new Rtcm1046(1046, data);
    std::bitset<8+16+504+24> bits { 0UL };
    for (auto b : data) {
        const std::bitset<8+16+504+24> bs {b};
        bits <<= 8;
        bits  |= bs;
    }

    std::size_t i = 8+16; 
    getdatafield(bits,i,  m->mType);
    if (m->mType != 1046) {
        ERRORF("RTCM 1046 message missmatched message number. should be '1046', was '%4d'", m->mType.value());
        ERRORF("bits: %s", bits.to_string().c_str());
        return std::make_unique<ErrorMessage>(1046, std::move(data));
    }
    getdatafield(bits,i,  m->prn               );
    getdatafield(bits,i,  m->week_number       );
    getdatafield(bits,i,  m->iod_nav           );
    getdatafield(bits,i,  m->sisa_index        );
    getdatafield(bits,i,  m->idot              );
    getdatafield(bits,i,  m->toc               );
    getdatafield(bits,i,  m->af2               );
    getdatafield(bits,i,  m->af1               );
    getdatafield(bits,i,  m->af0               );
    getdatafield(bits,i,  m->crs               );
    getdatafield(bits,i,  m->delta_n           );
    getdatafield(bits,i,  m->m0                );
    getdatafield(bits,i,  m->cuc               );
    getdatafield(bits,i,  m->e                 );
    getdatafield(bits,i,  m->cus               );
    getdatafield(bits,i,  m->sqrt_a            );
    getdatafield(bits,i,  m->toe               );
    getdatafield(bits,i,  m->cic               );
    getdatafield(bits,i,  m->omega0            );
    getdatafield(bits,i,  m->cis               );
    getdatafield(bits,i,  m->i0                );
    getdatafield(bits,i,  m->crc               );
    getdatafield(bits,i,  m->omega             );
    getdatafield(bits,i,  m->omega_dot         );
    getdatafield(bits,i,  m->BGD_E5a_E1        );
    getdatafield(bits,i,  m->BGD_E5b_E1        );
    getdatafield(bits,i,  m->E5b_signal_health );
    getdatafield(bits,i,  m->E5b_data_validity );
    getdatafield(bits,i,  m->E1_B_signal_health);
    getdatafield(bits,i,  m->E1_B_data_validity);
    getdatafield(bits,i,  m->reserved          );
                             
    return std::unique_ptr<Rtcm1046>(m);
}

}  // namespace rtcm
}  // namespace format
