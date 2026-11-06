#pragma once
#include <format/rtcm/message.hpp>

#include <memory>

namespace format {
namespace rtcm {

/// GLONASS ephemeris (RTCM 10403.4 Table 3.5-21), values in ICD units (km, km/s, km/s², s)
class Rtcm1020 final : public Message {
public:
    EXPLICIT Rtcm1020(std::vector<uint8_t> data) NOEXCEPT;
    ~Rtcm1020() override = default;

    Rtcm1020(Rtcm1020 const& other)      = delete;
    Rtcm1020(Rtcm1020&&)                 = delete;
    Rtcm1020& operator=(Rtcm1020 const&) = delete;
    Rtcm1020& operator=(Rtcm1020&&)      = delete;

    void      print() const NOEXCEPT override;
    NODISCARD std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(std::vector<uint8_t> data);

    uint8_t  slot;                      // DF038
    int8_t   frequency_channel;         // DF040 - 7
    bool     almanac_health;            // DF104
    bool     almanac_health_available;  // DF105
    uint8_t  p1;                        // DF106
    uint8_t  tk_h;                      // DF107
    uint8_t  tk_m;
    uint8_t  tk_s;
    bool     bn_msb;           // DF108
    bool     p2;               // DF109
    uint8_t  tb;               // DF110, 15 minute index of the Moscow time of day
    double   velocity[3];      // DF111/115/119
    double   position[3];      // DF112/116/120
    double   acceleration[3];  // DF113/117/121
    bool     p3;               // DF120
    double   gamma_n;          // DF121
    uint8_t  p;                // DF122
    bool     ln3;              // DF123
    double   tau_n;            // DF124
    double   delta_tau_n;      // DF125
    uint8_t  e_n;              // DF126, age in days
    bool     p4;               // DF127
    uint8_t  f_t;              // DF128
    uint16_t n_t;              // DF129, day in the four-year interval (1..1461), 0 = unknown
    uint8_t  m;                // DF130
    bool     additional_data;  // DF131
    uint16_t n_a;              // DF132
    double   tau_c;            // DF133
    uint8_t  n_4;              // DF134, four-year interval since 1996 (1..31), 0 = unknown
    double   tau_gps;          // DF135
    bool     ln5;              // DF136
};

}  // namespace rtcm
}  // namespace format
