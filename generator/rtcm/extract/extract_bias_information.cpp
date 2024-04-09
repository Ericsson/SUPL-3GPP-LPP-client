#include "extract.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <GLO-RTK-BiasInformation-r15.h>
#pragma GCC diagnostic pop

#include <asn.1/bit_string.hpp>

using namespace generator::rtcm;

namespace decode {
static uint32_t reference_station_id(GLO_RTK_BiasInformation_r15 const& src_bias_info) {
    return static_cast<uint32_t>(src_bias_info.referenceStationID_r15.referenceStationID_r15);
}

static uint8_t mask(GLO_RTK_BiasInformation_r15 const& src_bias_info) {
    return (src_bias_info.l1_ca_cpBias_r15 ? 0x01 : 0x00) |
           (src_bias_info.l1_p_cpBias_r15 ? 0x02 : 0x00) |
           (src_bias_info.l2_ca_cpBias_r15 ? 0x04 : 0x00) |
           (src_bias_info.l2_p_cpBias_r15 ? 0x08 : 0x00);
}

static double bias(long value) {
    return static_cast<double>(value) * 0.02;
}

static Maybe<double> l1_ca(GLO_RTK_BiasInformation_r15 const& src_bias_info) {
    if (src_bias_info.l1_ca_cpBias_r15) {
        return bias(*src_bias_info.l1_ca_cpBias_r15);
    } else {
        return Maybe<double>();
    }
}

static Maybe<double> l1_p(GLO_RTK_BiasInformation_r15 const& src_bias_info) {
    if (src_bias_info.l1_p_cpBias_r15) {
        return bias(*src_bias_info.l1_p_cpBias_r15);
    } else {
        return Maybe<double>();
    }
}

static Maybe<double> l2_ca(GLO_RTK_BiasInformation_r15 const& src_bias_info) {
    if (src_bias_info.l2_ca_cpBias_r15) {
        return bias(*src_bias_info.l2_ca_cpBias_r15);
    } else {
        return Maybe<double>();
    }
}

static Maybe<double> l2_p(GLO_RTK_BiasInformation_r15 const& src_bias_info) {
    if (src_bias_info.l2_p_cpBias_r15) {
        return bias(*src_bias_info.l2_p_cpBias_r15);
    } else {
        return Maybe<double>();
    }
}

static bool indicator(GLO_RTK_BiasInformation_r15 const& src_bias_info) {
    auto indicator = helper::BitString::from(&src_bias_info.cpbIndicator_r15);
    return indicator->get_bit(0) != 0;
}
}  // namespace decode

extern void extract_bias_information(RtkData&                           data,
                                     GLO_RTK_BiasInformation_r15 const& src_bias_info) {
    auto  dst_bias_info            = std::unique_ptr<BiasInformation>(new BiasInformation());
    auto& bias_info                = *dst_bias_info.get();
    bias_info.reference_station_id = decode::reference_station_id(src_bias_info);
    bias_info.mask                 = decode::mask(src_bias_info);
    bias_info.l1_ca                = decode::l1_ca(src_bias_info);
    bias_info.l1_p                 = decode::l1_p(src_bias_info);
    bias_info.l2_ca                = decode::l2_ca(src_bias_info);
    bias_info.l2_p                 = decode::l2_p(src_bias_info);
    bias_info.indicator            = decode::indicator(src_bias_info);

    data.bias_information = std::move(dst_bias_info);
}
