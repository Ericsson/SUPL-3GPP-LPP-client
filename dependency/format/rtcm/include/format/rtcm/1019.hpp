#pragma once
#include <format/rtcm/message.hpp>

#include <cmath>
#include <memory>

#include <time/tai.hpp>
#include <datafields.hpp>

namespace format {
namespace rtcm{

class RTCM1019Message final : public Message {
public:
    ~RTCM1019Message() override = default;

    RTCM1019Message(RTCM1019Message const& other)
        : Message(other) {}
    RTCM1019Message(RTCM1019Message&&)                 = delete;
    RTCM1019Message& operator=(RTCM1019Message const&) = delete;
    RTCM1019Message& operator=(RTCM1019Message&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(std::vector<uint8_t> data);

private:
    EXPLICIT RTCM1019Message(std::vector<uint8_t> data) NOEXCEPT;
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
};

}  // namespace rtcm
}  // namespace format
