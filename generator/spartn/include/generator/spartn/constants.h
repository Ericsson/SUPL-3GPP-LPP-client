#ifndef SPARTN_CONSTANTS
#define SPARTN_CONSTANTS

#include <cstdint>
#include <map>
#include <string>
#include <array>

namespace Constants {
static constexpr uint16_t max_payload_size     = 8192;
static constexpr uint16_t max_message_size     = 50115;
static constexpr uint8_t  max_spartn_bit_count = 58;

static constexpr uint8_t lpp_stec_qual_num     = 63;
static constexpr uint8_t spartn_stec_qual_num  = 14;
static constexpr uint8_t spartn_tropo_qual_num = 6;
static constexpr uint8_t spartn_continuity_num = 8;
static constexpr uint8_t num_of_gnss           = 7;
/*
 * All units are in a meters;
 */
static const std::map<long, uint8_t> gnss_id_to_hpac_subtype = {{0, 0}, {3, 2}, {4, 1}};

static constexpr double orbit_lpp_radial_res = 0.0001;
static constexpr double orbit_lpp_along_res  = 0.0004;
static constexpr double orbit_lpp_cross_res  = 0.0004;

static constexpr double clock_c0_lpp_res = 0.0001;
static constexpr double clock_c1_lpp_res = 0.000001;
static constexpr double clock_c2_lpp_res = 0.00000002;

static constexpr double orbit_clock_spartn_res     = 0.002;
static constexpr double orbit_clock_spartn_rng_min = -16.382;

static constexpr double bias_lpp_res              = 0.01;
static constexpr double bias_spartn_res           = 0.02;
static constexpr double code_bias_spartn_rng_min  = -20.46;
static constexpr double phase_bias_lpp_res        = 0.001;
static constexpr double phase_bias_spartn_res     = 0.002;
static constexpr double phase_bias_spartn_rng_min = -16.382;

static constexpr double iono_lpp_c00_res              = 0.05;
static constexpr double iono_lpp_c01_res              = 0.02;
static constexpr double iono_lpp_c10_res              = 0.02;
static constexpr double iono_lpp_c11_res              = 0.02;
static constexpr double iono_spartn_c00_res           = 0.04;
static constexpr double iono_spartn_c00_small_rng_min = -81.88;
static constexpr double iono_spartn_c00_large_rng_min = -327.64;
static constexpr double iono_spartn_c01_res           = 0.008;
static constexpr double iono_spartn_c01_small_rng_min = -16.376;
static constexpr double iono_spartn_c01_large_rng_min = -65.528;
static constexpr double iono_spartn_c10_res           = 0.008;
static constexpr double iono_spartn_c10_small_rng_min = -16.376;
static constexpr double iono_spartn_c10_large_rng_min = -65.528;
static constexpr double iono_spartn_c11_res           = 0.002;
static constexpr double iono_spartn_c11_small_rng_min = -8.190;
static constexpr double iono_spartn_c11_large_rng_min = -32.766;

static constexpr double tropo_dry_res            = 0.004;
static constexpr double tropo_dry_spartn_rng_min = -0.508;
static constexpr double tropo_dry_lpp_offset     = 2.3;
static constexpr double tropo_wet_res            = 0.004;
static constexpr double tropo_wet_spartn_rng_min = -1.020;

static constexpr double residual_res                        = 0.04;  // same for LPP & SPARTN
static constexpr double residual_spartn_small_rng_min       = -0.28;
static constexpr double residual_spartn_medium_rng_min      = -2.52;
static constexpr double residual_spartn_large_rng_min       = -20.44;
static constexpr double residual_spartn_extra_large_rng_min = -327.64;

static constexpr double delta_lpp_res        = 0.01;
static constexpr double delta_spartn_res     = 0.1;
static constexpr double delta_spartn_rng_min = 0.1;
static constexpr double delta_spartn_rng_max = 3.2;

static constexpr double wetdelay_res            = 0.004;
static constexpr double wetdelay_spartn_rng_min = -0.508;

static constexpr uint16_t two_to_the_fourteen = 16384;
static constexpr uint16_t two_to_the_fifteen  = 32768;
static constexpr uint8_t  deg_in_lat          = 90;
static constexpr uint8_t  deg_in_lng          = 180;
static constexpr double   lat_lng_res         = 0.1;
static constexpr double   step_lpp_res        = 0.01;
static constexpr double   step_spartn_res     = 0.1;

static constexpr uint32_t seconds_in_day         = 86400;
static constexpr uint16_t day_delta_1970_1980    = 3657;   // Jan 6
static constexpr uint16_t day_delta_1970_1996    = 9496;   // Jan 1
static constexpr uint16_t day_delta_1970_1999    = 10825;  // Aug 2
static constexpr uint16_t day_delta_1970_2006    = 13149;  // Jan 1
static constexpr uint16_t day_delta_1970_2010    = 14610;  // Jan 1
static constexpr uint32_t second_delta_1970_2010 = 1262304000;

// if a index of 64 is found, then there is no max...
static constexpr std::array<double, lpp_stec_qual_num> lpp_stec_qualities_maxiumums = {
    33.6664, 30.2992, 26.9319, 23.5647, 20.1974, 16.8301, 13.4629, 12.3405, 11.2180,
    10.0956, 8.9732,  7.8508,  6.7284,  5.6059,  4.4835,  4.1094,  3.7352,  3.3611,
    2.9870,  2.6128,  2.2387,  1.8645,  1.4904,  1.3657,  1.2410,  1.1163,  0.9915,
    0.8668,  0.7421,  0.6174,  0.4927,  0.4511,  0.4096,  0.3680,  0.3264,  0.2848,
    0.2433,  0.2017,  0.1601,  0.1463,  0.1324,  0.1186,  0.1047,  0.0908,  0.0770,
    0.0631,  0.0493,  0.0447,  0.0400,  0.0354,  0.0308,  0.0262,  0.0216,  0.0169,
    0.0123,  0.0108,  0.0092,  0.0077,  0.0062,  0.0046,  0.0031,  0.0015,  0};

static constexpr std::array<double, spartn_stec_qual_num> spartn_stec_qualities_maximums = {
    0.03, 0.05, 0.07, 0.14, 0.28, 0.56, 1.12, 2.24, 4.48, 8.96, 17.92, 35.84, 71.68, 143.36};

static constexpr std::array<double, spartn_tropo_qual_num> spartn_tropo_qualities_maximums = {
    0.010, 0.020, 0.040, 0.080, 0.160, 0.320};

static constexpr std::array<double, 8> ura_values = {0, 0.01, 0.02, 0.05, 0.1, 0.3, 1.0};

static constexpr std::array<double, 16> update_intervals = {
    1, 2, 5, 10, 15, 30, 60, 120, 240, 300, 600, 900, 1800, 3600, 7200, 10800};

static const std::map<long, uint8_t> gps_lpp_to_spartn_bias  = {{0, 0}, {8, 1}, {10, 2}, {13, 3}};
static const std::map<long, uint8_t> glo_lpp_to_spartn_bias  = {{0, 0}, {1, 1}};
static const std::map<long, uint8_t> gal_lpp_to_spartn_bias  = {{5, 0}, {22, 1}, {16, 2}};
static const std::map<long, uint8_t> bds_lpp_to_spartn_bias  = {{0, 0}, {6, 2}, {3, 3}};
static const std::map<long, uint8_t> qzss_lpp_to_spartn_bias = {{0, 0}, {8, 1}, {11, 2}};

static constexpr uint8_t max_bias_mask_bit_count = 9;

typedef struct bias_ids_size {
    uint8_t phase_id;
    uint8_t code_id;
    uint8_t bit_count;
} bias_ids_size;

static const bias_ids_size gps_bias_ids_size = {
    25,
    27,
    7,
};

static const bias_ids_size glo_bias_ids_size = {
    26,
    28,
    6,
};

static const bias_ids_size gal_bias_ids_size = {
    102,
    105,
    9,
};

static const bias_ids_size bds_bias_ids_size = {
    106,
    103,
    9,
};

static const bias_ids_size qzss_bias_ids_size = {
    107,
    104,
    7,
};

/*
Generated with:
    polynomial = 0x09U
    initial value = 0
    input reflected = true
    result reflected = true
    final XOR value = 0
*/
static const uint8_t crc4_lookuptable[256] = {
    0x00, 0x0B, 0x05, 0x0E, 0x0A, 0x01, 0x0F, 0x04, 0x07, 0x0C, 0x02, 0x09, 0x0D, 0x06, 0x08, 0x03,
    0x0E, 0x05, 0x0B, 0x00, 0x04, 0x0F, 0x01, 0x0A, 0x09, 0x02, 0x0C, 0x07, 0x03, 0x08, 0x06, 0x0D,
    0x0F, 0x04, 0x0A, 0x01, 0x05, 0x0E, 0x00, 0x0B, 0x08, 0x03, 0x0D, 0x06, 0x02, 0x09, 0x07, 0x0C,
    0x01, 0x0A, 0x04, 0x0F, 0x0B, 0x00, 0x0E, 0x05, 0x06, 0x0D, 0x03, 0x08, 0x0C, 0x07, 0x09, 0x02,
    0x0D, 0x06, 0x08, 0x03, 0x07, 0x0C, 0x02, 0x09, 0x0A, 0x01, 0x0F, 0x04, 0x00, 0x0B, 0x05, 0x0E,
    0x03, 0x08, 0x06, 0x0D, 0x09, 0x02, 0x0C, 0x07, 0x04, 0x0F, 0x01, 0x0A, 0x0E, 0x05, 0x0B, 0x00,
    0x02, 0x09, 0x07, 0x0C, 0x08, 0x03, 0x0D, 0x06, 0x05, 0x0E, 0x00, 0x0B, 0x0F, 0x04, 0x0A, 0x01,
    0x0C, 0x07, 0x09, 0x02, 0x06, 0x0D, 0x03, 0x08, 0x0B, 0x00, 0x0E, 0x05, 0x01, 0x0A, 0x04, 0x0F,
    0x09, 0x02, 0x0C, 0x07, 0x03, 0x08, 0x06, 0x0D, 0x0E, 0x05, 0x0B, 0x00, 0x04, 0x0F, 0x01, 0x0A,
    0x07, 0x0C, 0x02, 0x09, 0x0D, 0x06, 0x08, 0x03, 0x00, 0x0B, 0x05, 0x0E, 0x0A, 0x01, 0x0F, 0x04,
    0x06, 0x0D, 0x03, 0x08, 0x0C, 0x07, 0x09, 0x02, 0x01, 0x0A, 0x04, 0x0F, 0x0B, 0x00, 0x0E, 0x05,
    0x08, 0x03, 0x0D, 0x06, 0x02, 0x09, 0x07, 0x0C, 0x0F, 0x04, 0x0A, 0x01, 0x05, 0x0E, 0x00, 0x0B,
    0x04, 0x0F, 0x01, 0x0A, 0x0E, 0x05, 0x0B, 0x00, 0x03, 0x08, 0x06, 0x0D, 0x09, 0x02, 0x0C, 0x07,
    0x0A, 0x01, 0x0F, 0x04, 0x00, 0x0B, 0x05, 0x0E, 0x0D, 0x06, 0x08, 0x03, 0x07, 0x0C, 0x02, 0x09,
    0x0B, 0x00, 0x0E, 0x05, 0x01, 0x0A, 0x04, 0x0F, 0x0C, 0x07, 0x09, 0x02, 0x06, 0x0D, 0x03, 0x08,
    0x05, 0x0E, 0x00, 0x0B, 0x0F, 0x04, 0x0A, 0x01, 0x02, 0x09, 0x07, 0x0C, 0x08, 0x03, 0x0D, 0x06};

/*!
    This table provides a lookup table for the CCITT CRC16 algorithm
    Generated with:
        polynomial = 0x1021U
        initial value = 0
        input reflected = false
        result reflected = false
        final XOR value = 0
 */
static const uint16_t lib_crc_kCrc16qtable[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B,
    0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 0x2462, 0x3443, 0x0420, 0x1401,
    0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738,
    0xF7DF, 0xE7FE, 0xD79D, 0xC7BC, 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 0x5AF5, 0x4AD4, 0x7AB7, 0x6A96,
    0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD,
    0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78, 0x9188, 0x81A9, 0xB1CA, 0xA1EB,
    0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2,
    0x4235, 0x5214, 0x6277, 0x7256, 0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xA7DB, 0xB7FA, 0x8799, 0x97B8,
    0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827,
    0x18C0, 0x08E1, 0x3882, 0x28A3, 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 0xFD2E, 0xED0F, 0xDD6C, 0xCD4D,
    0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74,
    0x2E93, 0x3EB2, 0x0ED1, 0x1EF0};

static constexpr std::array<uint32_t, spartn_continuity_num> allowed_iode_continuity_values = {
    0, 1, 5, 10, 30, 60, 120, 320};

static constexpr std::array<uint8_t, num_of_gnss> iode_ids = {18, 0, 101, 99, 19, 100, 0};

static constexpr std::array<uint8_t, num_of_gnss> iode_bit_size = {8, 0, 8, 10, 7, 8, 0};

static const std::array<std::string, num_of_gnss> constellation_names = {
    "GPS", "SBAS", "QZSS", "GAL", "GLO", "BDS", "NAVIC"};

static constexpr std::array<uint8_t, num_of_gnss> satellite_mask_ids = {11, 0, 0, 93, 12, 0, 0};

static constexpr std::array<uint8_t, num_of_gnss> satellite_mask_sizes = {58, 0, 0, 56, 50, 0, 0};

static constexpr std::array<int8_t, num_of_gnss> constellation_subtypes = {0, -1, -1, 2, 1, -1, -1};

static constexpr std::array<uint8_t, num_of_gnss> ephemeris_id = {16, 0, 0, 96, 17, 0, 0};

static constexpr std::array<uint8_t, num_of_gnss> ephemeris_bit_count = {2, 0, 0, 3, 2, 0, 0};

static constexpr std::array<uint8_t, num_of_gnss> ephemeris_bits = {0, 0, 0, 1, 0, 0, 0};
}  // namespace Constants

#endif
