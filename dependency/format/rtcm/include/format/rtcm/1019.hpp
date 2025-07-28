#pragma once
#include <format/rtcm/message.hpp>

#include <cmath>
#include <memory>

#include <time/tai.hpp>
#include "datafields.hpp"

// TODO: Put headers for messages in /messages folder
// Also maybe remove Message in Rtcm1019Message to be more uniform with other formats

namespace format {
namespace rtcm{

class Rtcm1019Message final : public Message {
public:
    ~Rtcm1019Message() override = default;

    Rtcm1019Message(Rtcm1019Message const& other)
        : Message(other) {}
    Rtcm1019Message(Rtcm1019Message&&)                 = delete;
    Rtcm1019Message& operator=(Rtcm1019Message const&) = delete;
    Rtcm1019Message& operator=(Rtcm1019Message&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(std::vector<uint8_t> data);

    DF009  prn;
    DF076  week;
    DF077  SV_ACCURACY;
    DF078  code_on_l2;
    DF079  idot;
    DF071  iode;
    DF081  t_oc;
    DF082  a_f2;
    DF083  a_f1;
    DF084  a_f0;
    DF085  iodc;
    DF086  C_rs;
    DF087  dn;
    DF088  M_0;
    DF089  C_uc;
    DF090  e;
    DF091  C_us;
    DF092  sqrt_A;
    DF093  t_oe;
    DF094  C_ic;
    DF095  OMEGA_0;
    DF096  C_is;
    DF097  i_0;
    DF098  C_rc;
    DF099  omega;
    DF100  OMEGADOT;
    DF101  t_GD;
    DF102  SV_HEALTH;
    DF103  L2_P_data_flag;
    DF137  fit;
private:
    EXPLICIT Rtcm1019Message(std::vector<uint8_t> data) NOEXCEPT;
};

}  // namespace rtcm
}  // namespace format
