#pragma once
#include <core/core.hpp>
#include <format/nav/words.hpp>

#include <unordered_map>

namespace ephemeris {
struct BdsEphemeris;
}

namespace format {
namespace nav {

struct D1Subframe1 {
    uint32_t sow;
    uint8_t  sat_h1;
    uint8_t  aodc;
    uint8_t  urai;
    uint16_t wn;
    double   toc;
    double   tgd1;
    double   tgd2;
    double   iono_alpha[4];
    double   iono_beta[4];
    double   a0;
    double   a1;
    double   a2;
    uint8_t  aode;
};

struct D1Subframe2 {
    uint32_t sow;
    double   delta_n;
    double   cuc;
    double   m0;
    double   e;
    double   cus;
    double   crc;
    double   crs;
    double   sqrt_a;
    uint32_t toe_msb;
};

struct D1Subframe3 {
    uint32_t sow;
    uint32_t toe_lsb;
    double   i0;
    double   cic;
    double   omega_dot;
    double   cis;
    double   idot;
    double   omega0;
    double   omega;
};

struct D1Subframe {
    uint8_t subframe_id;
    uint8_t sv_id;

    union {
        D1Subframe1 subframe1;
        D1Subframe2 subframe2;
        D1Subframe3 subframe3;
    };

    NODISCARD static bool decode(Words const& words, uint8_t sv_id, D1Subframe& subframe) NOEXCEPT;
};

class D1Collector {
public:
    D1Collector() NOEXCEPT = default;

    bool process(uint8_t prn, D1Subframe const& subframe,
                 ephemeris::BdsEphemeris& ephemeris) NOEXCEPT;

private:
    struct InternalEphemeris {
        bool subframe1;
        bool subframe2;
        bool subframe3;

        D1Subframe1 subframe1_data;
        D1Subframe2 subframe2_data;
        D1Subframe3 subframe3_data;
    };

    std::unordered_map<uint8_t, InternalEphemeris> mBuffer;
};

}  // namespace nav
}  // namespace format
