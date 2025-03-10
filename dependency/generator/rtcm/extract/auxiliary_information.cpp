#include "extract.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <GNSS-AuxiliaryInformation.h>
#include <GNSS-ID-GLONASS-SatElement.h>
#include <GNSS-ID-GLONASS.h>
#pragma GCC diagnostic pop

using namespace generator::rtcm;

namespace decode {
static SatelliteId satellite_id(GenericGnssId                     gnss_id,
                                GNSS_ID_GLONASS_SatElement const& src_satellite) {
    auto gnss = SatelliteId::Gnss::UNKNOWN;
    switch (gnss_id) {
    case GenericGnssId::GPS: gnss = SatelliteId::Gnss::GPS; break;
    case GenericGnssId::GLONASS: gnss = SatelliteId::Gnss::GLONASS; break;
    case GenericGnssId::GALILEO: gnss = SatelliteId::Gnss::GALILEO; break;
    case GenericGnssId::BEIDOU: gnss = SatelliteId::Gnss::BEIDOU; break;
    }

    auto id = src_satellite.svID.satellite_id;
    return SatelliteId::from_lpp(gnss, id);
}

static Maybe<int32_t> frequency_channel(GNSS_ID_GLONASS_SatElement const& src_satellite) {
    if (src_satellite.channelNumber) {
        return static_cast<int32_t>(*src_satellite.channelNumber);
    } else {
        return Maybe<int32_t>();
    }
}
}  // namespace decode

static std::unique_ptr<AuxiliaryInformation>
extract_glonass_auxiliary_information(const GNSS_ID_GLONASS& src_glonass) {
    auto  dst_aux = std::unique_ptr<AuxiliaryInformation>(new AuxiliaryInformation());
    auto& aux     = *dst_aux.get();

    auto& list = src_glonass.list;
    for (auto i = 0; i < list.count; i++) {
        if (!list.array[i]) continue;
        auto& element = *list.array[i];

        auto id                = decode::satellite_id(GenericGnssId::GLONASS, element);
        auto frequency_channel = decode::frequency_channel(element);

        AuxiliaryInformation::Satellite satellite{};
        satellite.id                 = id;
        satellite.frequency_channel  = frequency_channel;
        aux.satellites[satellite.id] = satellite;
    }

    return dst_aux;
}

extern void extract_auxiliary_information(RtkData& data, GNSS_AuxiliaryInformation const& src_aux) {
    switch (src_aux.present) {
    case GNSS_AuxiliaryInformation_PR_gnss_ID_GLONASS:
        data.auxiliary_information =
            extract_glonass_auxiliary_information(src_aux.choice.gnss_ID_GLONASS);
        break;
    case GNSS_AuxiliaryInformation_PR_gnss_ID_GPS:
    case GNSS_AuxiliaryInformation_PR_ext1:
    case GNSS_AuxiliaryInformation_PR_NOTHING: break;
    }
}
