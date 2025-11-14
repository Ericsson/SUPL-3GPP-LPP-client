#pragma once
#include <format/rtcm/message.hpp>

#include <cmath>
#include <memory>

#include <time/tai.hpp>
#include "datafields.hpp"

// TODO: Put headers for messages in /messages folder
// Also maybe remove Message in Rtcm1019Message to be more uniform with other formats

namespace format {
namespace rtcm {

class Rtcm1019 final : public Message {
public:
    EXPLICIT Rtcm1019(DF002 type, std::vector<uint8_t> data) NOEXCEPT;
    ~Rtcm1019() override = default;

    Rtcm1019(Rtcm1019 const& other) : Message(other) {}
    Rtcm1019(Rtcm1019&&)                 = delete;
    Rtcm1019& operator=(Rtcm1019 const&) = delete;
    Rtcm1019& operator=(Rtcm1019&&)      = delete;

    void      print() const NOEXCEPT override;
    NODISCARD std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(std::vector<uint8_t> data);

    DF009 prn;
    DF076 week;
    DF077 sv_accuracy;
    DF078 code_on_l2;
    DF079 idot;
    DF071 iode;
    DF081 t_oc;
    DF082 a_f2;
    DF083 a_f1;
    DF084 a_f0;
    DF085 iodc;
    DF086 c_rs;
    DF087 delta_n;
    DF088 m_0;
    DF089 c_uc;
    DF090 e;
    DF091 c_us;
    DF092 sqrt_a;
    DF093 t_oe;
    DF094 c_ic;
    DF095 omega_0;
    DF096 c_is;
    DF097 i_0;
    DF098 c_rc;
    DF099 omega;
    DF100 omegadot;
    DF101 t_gd;
    DF102 sv_health;
    DF103 l2_p_data_flag;
    DF137 fit;

private:
    EXPLICIT Rtcm1019(std::vector<uint8_t> data) NOEXCEPT;
};

}  // namespace rtcm
}  // namespace format
