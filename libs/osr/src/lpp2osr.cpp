#include "osr/lpp2osr.h"
#include "utility/satellite_id.h"

#include <asn_helper.h>
#include <rtklib.h>

#define LEAPSECONDS_FROM_UTC (37 - 19)

#define P2_M4 0.0625
#define P2_M10 9.765625E-4
#define P2_M29 1.862645149230957E-9
#define P2_M31 4.6566128730773926E-10

constexpr static int      mlt_size                  = 22;
constexpr static uint64_t mlt_coefficient[mlt_size] = {
    1,    2,    4,    8,     16,    32,    64,     128,    256,    512,     1024,
    2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152,
};

constexpr static uint64_t mlt_offset[mlt_size] = {
    0,        64,       256,       768,       2048,      5120,       12288,   28672,
    65536,    147456,   327680,    720896,    1572864,   3407872,    7340032, 15728640,
    33554432, 71303168, 150994944, 318767104, 671088640, 1409286144,
};

constexpr static uint64_t mlt_base[mlt_size] = {
    0,   64,  96,  128, 160, 192, 224, 256, 288, 320, 352,
    384, 416, 448, 480, 512, 544, 576, 608, 640, 672, 704,
};

// Table 3.5-75 GNSS Phaserange Lock Time Indicator with Extended Range and
// Resolution (DF407)
double from_msm_lock_ex(long value) {
    if ((uint64_t)value >= mlt_base[mlt_size - 1]) {
        return 67108864 / 1000.0;
    }

    for (auto i = 0; i < mlt_size - 1; i++) {
        auto start = mlt_base[i];
        auto end   = mlt_base[i + 1];
        if ((uint64_t)value >= start && (uint64_t)value < end) {
            return (mlt_coefficient[i] * value - mlt_offset[i]) / 1000.0;
        }
    }

    return 0.0;
}

constexpr static uint64_t mlt2_table[16] = {
    0, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288,
};

double from_msm_lock(long value) {
    if (value < 0) return 0;
    if (value > 15) return mlt2_table[15] / 1000.0;
    return mlt2_table[value] / 1000.0;
}

static const double glt0[] = {1995, 12, 31, 21, 0, 0};
static gtime_t      glt2time(int week, double sec) {
         gtime_t t = epoch2time(glt0);

         if (sec < -1E9 || 1E9 < sec) sec = 0.0;
    t.time += (time_t)86400 * 7 * week + (int)sec;
         t.sec = sec - (int)sec;
         return t;
}

static GPS_Time gps_time_from(GNSS_SystemTime_t* sys_time, long leap_seconds) {
    long week = sys_time->gnss_DayNumber / 7;
    long day  = sys_time->gnss_DayNumber % 7;
    long tow  = day * 86400 + sys_time->gnss_TimeOfDay;

    if (sys_time->gnss_TimeOfDayFrac_msec) tow += *sys_time->gnss_TimeOfDayFrac_msec;

    gtime_t gtime;
    if (sys_time->gnss_TimeID.gnss_id == GNSS_ID__gnss_id_gps) {
        gtime = gpst2time(week, tow);
    } else if (sys_time->gnss_TimeID.gnss_id == GNSS_ID__gnss_id_glonass) {
        // NOTE: GLONASS is the only time system that is based on
        // UTC and thus includes leap-seconds.
        gtime = glt2time(week, tow + leap_seconds);
    } else if (sys_time->gnss_TimeID.gnss_id == GNSS_ID__gnss_id_galileo) {
        gtime = gst2time(week, tow);
    } else if (sys_time->gnss_TimeID.gnss_id == GNSS_ID__gnss_id_bds) {
        gtime = bdt2gpst(bdt2time(week, tow));
    }

    return GPS_Time{
        .time = gtime.time,
        .sec  = gtime.sec,
    };
}

bool gather_reference_station(LPP_Message* message, OSR* osr) {
    if (!message) return false;
    if (!message->lpp_MessageBody) return false;

    auto body     = message->lpp_MessageBody;
    auto pad      = &body->choice.c1.choice.provideAssistanceData;
    auto pad9     = &pad->criticalExtensions.choice.c1.choice.provideAssistanceData_r9;
    auto gnss_pad = pad9->a_gnss_ProvideAssistanceData;

    if (!gnss_pad) return false;
    if (!gnss_pad->gnss_CommonAssistData) return false;
    if (!gnss_pad->gnss_CommonAssistData->ext1) return false;

    auto gnss_RTK_CommonObservationInfo_r15 =
        gnss_pad->gnss_CommonAssistData->ext1->gnss_RTK_CommonObservationInfo_r15;
    if (gnss_RTK_CommonObservationInfo_r15) {
        auto common = &osr->common_observation.set_value();
        common->rtcm_id =
            gnss_RTK_CommonObservationInfo_r15->referenceStationID_r15.referenceStationID_r15;
        common->clock_steering = gnss_RTK_CommonObservationInfo_r15->clockSteeringIndicator_r15;
        common->external_clock = gnss_RTK_CommonObservationInfo_r15->externalClockIndicator_r15;
        common->smooth_indicator =
            BitString::from(&gnss_RTK_CommonObservationInfo_r15->smoothingIndicator_r15)
                ->get_bit(0);
        common->smooth_interval =
            (long)BitString::from(&gnss_RTK_CommonObservationInfo_r15->smoothingInterval_r15)
                ->as_int64();
    }

    auto rsi = gnss_pad->gnss_CommonAssistData->ext1->gnss_RTK_ReferenceStationInfo_r15;
    if (!rsi) return false;

    auto id = rsi->referenceStationID_r15.referenceStationID_r15;
    // TODO(ewasjon): auto name = rsi->referenceStationID_r15.providerName_r15;

    intmax_t point[3];
    asn_INTEGER2imax(&rsi->antenna_reference_point_ECEF_X_r15, &point[0]);
    asn_INTEGER2imax(&rsi->antenna_reference_point_ECEF_Y_r15, &point[1]);
    asn_INTEGER2imax(&rsi->antenna_reference_point_ECEF_Z_r15, &point[2]);

    RTK_ReferenceStation ref{};
    ref.id = id;
    ref.x  = point[0] * 0.0001;
    ref.y  = point[1] * 0.0001;
    ref.z  = point[2] * 0.0001;

    if (rsi->antennaHeight_r15) {
        ref.antenna_height = (*rsi->antennaHeight_r15) * 0.0001;
    }

    if (rsi->referenceStationIndicator_r15 ==
        GNSS_RTK_ReferenceStationInfo_r15__referenceStationIndicator_r15_physical)
        ref.reference_station_is_physical = true;
    else
        ref.reference_station_is_physical = false;

    // if it is non physical set non physical id
    if (!ref.reference_station_is_physical) {
        ref.non_physical_ref_sta_id = id;
    }

    // Added by Jacob, get physical ref station id
    auto physical_ref_station_info = rsi->physical_reference_station_info_r15;
    if (physical_ref_station_info) {
        intmax_t physical_points[3];
        asn_INTEGER2imax(&physical_ref_station_info->physical_ARP_ECEF_X_r15, &physical_points[0]);
        asn_INTEGER2imax(&physical_ref_station_info->physical_ARP_ECEF_Y_r15, &physical_points[1]);
        asn_INTEGER2imax(&physical_ref_station_info->physical_ARP_ECEF_Z_r15, &physical_points[2]);

        RTK_PhysicalReferenceStation pref{};
        pref.id = physical_ref_station_info->physicalReferenceStationID_r15.referenceStationID_r15;
        pref.x  = physical_points[0] * 0.0001;
        pref.y  = physical_points[1] * 0.0001;
        pref.z  = physical_points[2] * 0.0001;

        ref.physical = pref;
    }

    osr->reference_station                 = ref;
    osr->last_update_had_reference_station = true;
    return true;
}

static GNSS_System gnss_system_from_rtklib(int system) {
    if (system == SYS_GPS) return GNSS_System::GPS;
    if (system == SYS_GLO) return GNSS_System::GLONASS;
    if (system == SYS_GAL) return GNSS_System::GALILEO;
    if (system == SYS_CMP) return GNSS_System::BDS;
    assert(false);
    return GNSS_System::UNKNOWN;
}

static bool gather_satellite(OSR* osr, GNSS_RTK_SatelliteDataElement_r15_t* src, OSR_GNSS* gnss) {
    if (!src) return false;
    if (gnss->satellite_count >= OSR_MAX_SATELLITES) return false;

    auto dst = osr_get_or_allocate_satellite(osr, gnss, src->svID_r15.satellite_id);
    gnss->satellites[gnss->satellite_count++] = dst;

    dst->system                 = gnss->system;
    dst->id                     = src->svID_r15.satellite_id;
    dst->signal_count           = 0;
    dst->integer_ms             = {};
    dst->rough_range            = {};
    dst->rough_phase_range_rate = {};
    dst->glo_frequency_channel  = {};

    if (src->integer_ms_r15) {
        dst->integer_ms = *(src->integer_ms_r15);
    }

    dst->rough_range = src->rough_range_r15 * P2_M10 /* 2^-10 */;

    if (src->rough_phase_range_rate_r15) {
        dst->rough_phase_range_rate = *(src->rough_phase_range_rate_r15);
    }

    for (auto i = 0; i < src->gnss_rtk_SatelliteSignalDataList_r15.list.count; i++) {
        auto src_sig = src->gnss_rtk_SatelliteSignalDataList_r15.list.array[i];
        if (!src_sig) continue;

        auto signal_id = SignalID::from_lpp(gnss_system_from_rtklib(gnss->system),
                                            gnss2long(src_sig->gnss_SignalID_r15));
        if (!signal_id.initialized()) {
            continue;
        }

        if (dst->signal_count >= OSR_MAX_SIGNALS) continue;

        auto dst_sig                      = osr_get_or_allocate_signal(osr, dst, signal_id.value());
        dst->signals[dst->signal_count++] = dst_sig;

        dst_sig->id                          = signal_id.value();
        dst_sig->fine_PhaseRange             = {};
        dst_sig->fine_PseudoRange            = {};
        dst_sig->lockTimeIndicator           = {};
        dst_sig->carrier_to_noise_ratio      = {};
        dst_sig->halfCycleAmbiguityIndicator = {};
        dst_sig->fine_PhaseRangeRate         = {};

        dst_sig->fine_PseudoRange  = src_sig->fine_PseudoRange_r15 * P2_M29 /* 2^-29 */;
        dst_sig->fine_PhaseRange   = src_sig->fine_PhaseRange_r15 * P2_M31 /* 2^-31 */;
        dst_sig->lockTimeIndicator = from_msm_lock_ex(src_sig->lockTimeIndicator_r15);

        if (src_sig->carrier_to_noise_ratio_r15) {
            dst_sig->carrier_to_noise_ratio =
                *(src_sig->carrier_to_noise_ratio_r15) * P2_M4 /* 2^-4 */;
        }

        auto halfcycle = BitString::from(&src_sig->halfCycleAmbiguityIndicator_r15);
        dst_sig->halfCycleAmbiguityIndicator = halfcycle->get_bit(0);

        if (src_sig->fine_PhaseRangeRate_r15) {
            dst_sig->fine_PhaseRangeRate = *(src_sig->fine_PhaseRangeRate_r15) * 0.0001;
        }
    }

    return true;
}

static bool gather_auxiliary_information(OSR_GNSS* gnss, GNSS_AuxiliaryInformation* aux) {
    if (aux->present == GNSS_AuxiliaryInformation_PR_gnss_ID_GPS) {
        if (gnss->system != SYS_GPS) return false;
        // TODO(ewasjon):
        return false;
    } else if (aux->present == GNSS_AuxiliaryInformation_PR_gnss_ID_GLONASS) {
        if (gnss->system != SYS_GLO) return false;
        auto list = aux->choice.gnss_ID_GLONASS.list;
        for (auto i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            auto id        = element->svID.satellite_id;
            auto satellite = osr_get_satellite(gnss, id);
            if (satellite) {
                if (element->channelNumber) {
                    satellite->glo_frequency_channel = *element->channelNumber;
                }
            }
        }

        return true;
    } else if (aux->present == GNSS_AuxiliaryInformation_PR_ext1) {
        // TODO(ewasjon):
        return false;
    }

    return false;
}

static bool gather_gnss_element(OSR* osr, GNSS_GenericAssistDataElement* element, OSR_GNSS* gnss) {
    if (!gnss) return false;

    if (element->ext2) {
        auto observations = element->ext2->gnss_RTK_Observations_r15;
        if (observations) {
            gnss->time = gps_time_from(&observations->epochTime_r15, LEAPSECONDS_FROM_UTC);

            auto list = &observations->gnss_ObservationList_r15.list;
            for (auto i = 0; i < list->count; i++) {
                gather_satellite(osr, list->array[i], gnss);
            }
        }

        auto bias = element->ext2->glo_RTK_BiasInformation_r15;
        if (bias && gnss->system == SYS_GLO) {
            RTK_GLO_BiasInformation gbi{};
            gbi.reference_station_id = bias->referenceStationID_r15.referenceStationID_r15;
            gbi.mask = (bias->l1_ca_cpBias_r15 ? 1 : 0) | (bias->l1_p_cpBias_r15 ? 2 : 0) |
                       (bias->l2_ca_cpBias_r15 ? 4 : 0) | (bias->l2_p_cpBias_r15 ? 8 : 0);

            gbi.l1_ca = long_pointer(bias->l1_ca_cpBias_r15, 0) * 0.02;
            gbi.l1_p  = long_pointer(bias->l1_p_cpBias_r15, 0) * 0.02;
            gbi.l2_ca = long_pointer(bias->l2_ca_cpBias_r15, 0) * 0.02;
            gbi.l2_p  = long_pointer(bias->l2_p_cpBias_r15, 0) * 0.02;

            auto indicator = BitString::from(&bias->cpbIndicator_r15);
            gbi.indicator  = indicator->get_bit(0);

            gnss->bias_information = gbi;
        }

        auto residuals_r15 = element->ext2->gnss_RTK_Residuals_r15;
        if (residuals_r15) {
            RTK_Residuals residuals = {
                .system = gnss->system,
                .time   = gps_time_from(&residuals_r15->epochTime_r15, LEAPSECONDS_FROM_UTC),
                .reference_station_id =
                    residuals_r15->referenceStationID_r15.referenceStationID_r15,
                .n_refs = residuals_r15->n_Refs_r15,
            };

            auto list                         = &residuals_r15->rtk_residuals_list_r15.list;
            residuals.residual_elements_count = list->count;
            for (auto i = 0; i < list->count; i++) {
                auto residual = list->array[i];
                if (!residual) continue;

                Optional<long> id{};
                if (gnss->system == SYS_GPS) {
                    auto satellite =
                        SatelliteID::from_lpp(GNSS_System::GPS, residual->svID_r15.satellite_id);
                    if (!satellite.initialized()) continue;

                    id = satellite.value().as_df009();
                }
                if (gnss->system == SYS_GLO) {
                    auto satellite = SatelliteID::from_lpp(GNSS_System::GLONASS,
                                                           residual->svID_r15.satellite_id);
                    if (!satellite.initialized()) continue;

                    id = satellite.value().as_df038();
                }

                if (!id.initialized()) continue;

                residuals.list[i] = {
                    .satellite_id = id.value(),
                    .soc          = residual->s_oc_r15 * 0.5,
                    .sod          = residual->s_od_r15 * 0.01,
                    .soh          = residual->s_oh_r15 * 0.1,
                    .slc          = residual->s_lc_r15 * 0.5,
                    .sld          = residual->s_ld_r15 * 0.01,
                };
            }

            gnss->residuals = residuals;
        }
    }

    // Must happen after gather_satellite
    auto aux = element->gnss_AuxiliaryInformation;
    if (aux) {
        gather_auxiliary_information(gnss, aux);
    }

    return true;
}

bool gather_observations(LPP_Message* message, OSR* osr) {
    if (!message) return false;
    if (!message->lpp_MessageBody) return false;

    auto body     = message->lpp_MessageBody;
    auto pad      = &body->choice.c1.choice.provideAssistanceData;
    auto pad9     = &pad->criticalExtensions.choice.c1.choice.provideAssistanceData_r9;
    auto gnss_pad = pad9->a_gnss_ProvideAssistanceData;
    if (!gnss_pad) return false;

    auto generic = gnss_pad->gnss_GenericAssistData;
    if (!generic) return false;

    for (auto i = 0; i < generic->list.count; i++) {
        auto element = generic->list.array[i];
        if (!element) continue;

        OSR_GNSS* gnss = NULL;
        if (element->gnss_ID.gnss_id == GNSS_ID__gnss_id_gps) {
            gnss = &osr->gps;
        } else if (element->gnss_ID.gnss_id == GNSS_ID__gnss_id_glonass) {
            gnss = &osr->glo;
        } else if (element->gnss_ID.gnss_id == GNSS_ID__gnss_id_galileo) {
            gnss = &osr->gal;
        } else if (element->gnss_ID.gnss_id == GNSS_ID__gnss_id_bds) {
            gnss = &osr->bds;
        } else
            continue;

        gather_gnss_element(osr, element, gnss);
    }

    return true;
}
