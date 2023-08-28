#include "extract.hpp"

#include <GNSS-AuxiliaryInformation.h>
#include <GNSS-ID-GLONASS-SatElement.h>
#include <GNSS-ID-GLONASS.h>

using namespace generator::rtcm;

namespace decode {
static SatelliteId satellite_id(GenericGnssId                     gnss_id,
                                const GNSS_ID_GLONASS_SatElement& src_satellite) {
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

static Maybe<int32_t> frequency_channel(const GNSS_ID_GLONASS_SatElement& src_satellite) {
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
        satellite.id                = id;
        satellite.frequency_channel = frequency_channel;
        aux.satellites[satellite.id] = satellite;
    }

    return dst_aux;
}

extern void extract_auxiliary_information(RtkData& data, const GNSS_AuxiliaryInformation& src_aux) {
    switch (src_aux.present) {
    case GNSS_AuxiliaryInformation_PR_gnss_ID_GLONASS:
        data.auxiliary_information =
            extract_glonass_auxiliary_information(src_aux.choice.gnss_ID_GLONASS);
        break;
    default: break;
    }
}
