#pragma once
#include <core/core.hpp>
#include <format/nav/words.hpp>

#include <unordered_map>

namespace ephemeris {
struct GalEphemeris;
}

namespace format {
namespace nav {
namespace gal {

struct InavWord {
    struct WordType1 {
        uint16_t iod_nav;
        double   toe;
        double   m0;
        double   e;
        double   sqrt_a;
    };

    struct WordType2 {
        uint16_t iod_nav;
        double   omega0;
        double   i0;
        double   omega;
        double   idot;
    };

    struct WordType3 {
        uint16_t iod_nav;
        double   omega_dot;
        double   delta_n;
        double   cuc;
        double   cus;
        double   crc;
        double   crs;
    };

    struct WordType4 {
        uint16_t iod_nav;
        uint8_t  sv_id;
        double   cic;
        double   cis;
        double   toc;
        double   af0;
        double   af1;
        double   af2;
    };

    struct WordType5 {
        double   a_i0;
        double   a_i1;
        double   a_i2;
        bool     region1;
        bool     region2;
        bool     region3;
        bool     region4;
        bool     region5;
        double   bgd_e1_e5a;
        double   bgd_e1_e5b;
        uint8_t  e5a_hs;
        uint8_t  e1b_hs;
        uint8_t  e5b_dvs;
        uint8_t  e1b_dvs;
        uint16_t wn;
        uint32_t tow;
    };

    uint8_t word_type;
    union {
        WordType1 type1;
        WordType2 type2;
        WordType3 type3;
        WordType4 type4;
        WordType5 type5;
    };

    NODISCARD static bool decode(Words const& words, InavWord& word) NOEXCEPT;
};

class InavEphemerisCollector {
public:
    InavEphemerisCollector() = default;

    bool process(uint8_t prn, InavWord const& word, ephemeris::GalEphemeris& ephemeris) NOEXCEPT;

private:
    struct InternalEphemeris {
        bool word1;
        bool word2;
        bool word3;
        bool word4;
        bool word5;

        InavWord::WordType1 word1_data;
        InavWord::WordType2 word2_data;
        InavWord::WordType3 word3_data;
        InavWord::WordType4 word4_data;
        InavWord::WordType5 word5_data;
    };

    std::unordered_map<uint8_t, InternalEphemeris> mBuffer;
};

}  // namespace gal
}  // namespace nav
}  // namespace format
