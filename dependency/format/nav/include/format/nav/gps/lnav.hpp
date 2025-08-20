#pragma once
#include <core/core.hpp>
#include <format/nav/words.hpp>

#include <unordered_map>

namespace ephemeris {
struct GpsEphemeris;
}

namespace format {
namespace nav {
namespace gps {
namespace lnav {

struct Tlm {
    uint8_t  preamble;
    uint16_t tlm;
    bool     integrity_status_flag;
    bool     reserved;
};

struct How {
    uint32_t tow;
    bool     alert_flag;
    bool     anti_spoofing_flag;
    uint8_t  subframe_id;
};

struct Subframe1 {
    uint16_t week_number;
    uint8_t  ca_or_p_on_l2;
    uint8_t  ura_index;
    uint8_t  sv_health;
    uint16_t iodc;
    bool     l2_p_data_flag;
    double   tgd;
    double   toc;
    double   af2;
    double   af1;
    double   af0;
};

struct Subframe2 {
    uint8_t iode;
    double  crs;
    double  delta_n;
    double  m0;
    double  cuc;
    double  e;
    double  cus;
    double  sqrt_a;
    double  toe;
    bool    fit_interval_flag;
    uint8_t aodo;
};

struct Subframe3 {
    double  cic;
    double  omega0;
    double  cis;
    double  i0;
    double  crc;
    double  omega;
    double  omega_dot;
    uint8_t iode;
    double  idot;
};

struct Subframe4 {
    struct Page18 {
        // Ionospheric parameters
        double a[4];
        double b[4];
        // UTC parameters
        double A0;
        double A1;
        double delta_t_ls;
        double t_ot;
        double wn_t;
        double wn_lsf;
        double dn;
        double delta_t_lsf;
    };

    uint8_t data_id;
    uint8_t sv_id;

    union {
        Subframe4::Page18 page18;
    };
};

struct Subframe {
    Tlm tlm;
    How how;

    union {
        Subframe1 subframe1;
        Subframe2 subframe2;
        Subframe3 subframe3;
        Subframe4 subframe4;
    };

    NODISCARD static bool decode(Words const& words, Subframe& subframe) NOEXCEPT;
};

// Ephemeris takes LNAV subframes and processes them into a complete ephemeris
class EphemerisCollector {
public:
    EphemerisCollector() NOEXCEPT = default;

    bool process(uint8_t prn, Subframe const& subframe,
                 ephemeris::GpsEphemeris& ephemeris) NOEXCEPT;

private:
    struct InternalEphemeris {
        bool subframe1;
        bool subframe2;
        bool subframe3;

        Subframe1 subframe1_data;
        Subframe2 subframe2_data;
        Subframe3 subframe3_data;
    };

    std::unordered_map<uint8_t, InternalEphemeris> mBuffer;
};

}  // namespace lnav
}  // namespace gps
}  // namespace nav
}  // namespace format
