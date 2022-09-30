#include "osr2rtcm.h"
#include <osr/osr2rtklib.h>

static int get_rtcm_message_number_for_gnss_system(int sys, int msm_no) {
    int gps_start = 1070;
    int glo_start = 1080;
    int gal_start = 1090;
    int bds_start = 1120;
    switch (sys) {
    case SYS_GPS:
        return gps_start + msm_no;
    case SYS_GLO:
        return glo_start + msm_no;
    case SYS_GAL:
        return gal_start + msm_no;
    case SYS_CMP:
        return bds_start + msm_no;
    }
    return -1;
}

// Initilizes rtcm 3 message obj
rtcm_t* osr2rtcm_begin() {
    rtcm_t* rtcm   = (rtcm_t*)malloc(sizeof(rtcm_t));
    int     status = init_rtcm(rtcm);

    if (status != 1) {
        free(rtcm);
        rtcm = NULL;
    }

    return rtcm;
}

// frees rtcm 3 message obj
rtcm_t* osr2rtcm_end(rtcm_t* rtcm) {
    free_rtcm(rtcm);  // call free rtcm in RTKLIB
    free(rtcm);
    return NULL;
}

// generate rtcm3 1006 message
rtcm_t* gen_rtcm_1006(RTK_ReferenceStation sta, bool gps, bool glo, bool gal) {
    rtcm_t* rtcm = osr2rtcm_begin();

    if (rtcm != NULL) {
        rtcm->staid      = sta.id;
        rtcm->sta.pos[0] = sta.x;
        rtcm->sta.pos[1] = sta.y;
        rtcm->sta.pos[2] = sta.z;
        rtcm->sta.hgt    = sta.antenna_height.or_value(0.0);

        rtcm->sta.gnss_indicator_gps = gps ? 1 : 0;
        rtcm->sta.gnss_indicator_glo = glo ? 1 : 0;
        rtcm->sta.gnss_indicator_gal = gal ? 1 : 0;
        rtcm->sta.rs_indicator  = sta.reference_station_is_physical ? 0 : 1;
        rtcm->sta.sro_indicator = 0;
        rtcm->sta.qc_indicator  = 0;

        int status = gen_rtcm3(rtcm, 1006, 0, 0);
        if (status != 1) {
            rtcm = osr2rtcm_end(rtcm);
        }
    }

    return rtcm;
}

// generate rtcm3 1032 message
rtcm_t* gen_rtcm_1032(RTK_ReferenceStation         ref,
                      RTK_PhysicalReferenceStation sta) {
    rtcm_t* rtcm = osr2rtcm_begin();

    if (rtcm != NULL) {
        rtcm->phy_sta.non_physical_sta_id = ref.id;
        rtcm->phy_sta.physical_sta_id     = sta.id;
        rtcm->phy_sta.pos[0]              = sta.x;
        rtcm->phy_sta.pos[1]              = sta.y;
        rtcm->phy_sta.pos[2]              = sta.z;

        int status = gen_rtcm3(rtcm, 1032, 0, 0);
        if (status != 1) {
            rtcm = osr2rtcm_end(rtcm);
        }
    }

    return rtcm;
}

// generate rtcm3 1030 or 1031 message
rtcm_t* gen_gnss_residuals(RTK_Residuals* residuals) {
    rtcm_t* rtcm = osr2rtcm_begin();
    if (rtcm != NULL) {
        rtcm->time =
            gtime_t{.time = residuals->time.time, .sec = residuals->time.sec};
        rtcm->staid     = residuals->reference_station_id;
        rtcm->residuals = {
            .n_refs              = residuals->n_refs,
            .n_sat_sig_processed = residuals->residual_elements_count,
        };

        for (int ri = 0; ri < residuals->residual_elements_count; ri++) {
            auto& element = residuals->list[ri];
            auto  oc      = (long)(element.soc / 0.5);   // mm -> 0.5 mm
            auto  od      = (long)(element.sod / 0.01);  // ppm -> 0.01 ppm
            auto  oh      = (long)(element.soh / 0.1);   // ppm -> 0.1 ppm
            auto  lc      = (long)(element.slc / 0.5);   // mm -> 0.5 mm
            auto  ld      = (long)(element.sld / 0.01);  // ppm -> 0.01 ppm

            rtcm->residuals.list[ri] = {
                .satellite_id = residuals->list[ri].satellite_id,
                .soc          = oc,
                .sod          = od,
                .soh          = oh,
                .slc          = lc,
                .sld          = ld,
            };
        }

        int status = 0;
        if (residuals->system == SYS_GPS) {
            status = gen_rtcm3(rtcm, 1030, 0, 0);
        } else if (residuals->system == SYS_GLO) {
            status = gen_rtcm3(rtcm, 1031, 0, 0);
        } else {
            rtcm = osr2rtcm_end(rtcm);
            return rtcm;
        }

        if (status != 1) {
            rtcm = osr2rtcm_end(rtcm);
        }
    }

    return rtcm;
}

// generate rtcm3 MSM 4
rtcm_t* gen_rtcm_msm4(OSR_GNSS* gnss, CommonObservation common, int sync) {
    int rtcm_message_number =
        get_rtcm_message_number_for_gnss_system(gnss->system, 4);
    if (gnss->satellite_count <= 0) {
        return NULL;
    }

    rtcm_t* rtcm = osr2rtcm_begin();
    if (rtcm != NULL) {
        rtcm->staid = common.rtcm_id;
        rtcm->time  = gtime_t{.time = gnss->time.time, .sec = gnss->time.sec};
        rtcm->clock_steering   = OptInteger{(int)common.clock_steering, true};
        rtcm->external_clock   = OptInteger{(int)common.external_clock, true};
        rtcm->smooth_indicator = OptInteger{(int)common.smooth_indicator, true};
        rtcm->smooth_interval  = OptInteger{(int)common.smooth_interval, true};
        osr2rtklib_convert(rtcm, gnss->system, gnss->satellites,
                           gnss->satellite_count);

        if (rtcm_message_number == -1) {
            rtcm = osr2rtcm_end(rtcm);
            return rtcm;
        }

        int status = gen_rtcm3(rtcm, rtcm_message_number, 0, sync);
        if (status != 1) {
            rtcm = osr2rtcm_end(rtcm);
        }
    }

    return rtcm;
}

// generate rtcm3 MSM 5
rtcm_t* gen_rtcm_msm5(OSR_GNSS* gnss, CommonObservation common, int sync) {
    if (gnss->satellite_count <= 0)
        return NULL;

    rtcm_t* rtcm = osr2rtcm_begin();

    if (rtcm != NULL) {
        rtcm->staid = common.rtcm_id;
        rtcm->time  = gtime_t{.time = gnss->time.time, .sec = gnss->time.sec};
        rtcm->clock_steering   = OptInteger{(int)common.clock_steering, true};
        rtcm->external_clock   = OptInteger{(int)common.external_clock, true};
        rtcm->smooth_indicator = OptInteger{(int)common.smooth_indicator, true};
        rtcm->smooth_interval  = OptInteger{(int)common.smooth_interval, true};
        osr2rtklib_convert(rtcm, gnss->system, gnss->satellites,
                           gnss->satellite_count);

        int rtcm_message_number =
            get_rtcm_message_number_for_gnss_system(gnss->system, 5);
        if (rtcm_message_number == -1) {
            rtcm = osr2rtcm_end(rtcm);
            return rtcm;
        }

        int status = gen_rtcm3(rtcm, rtcm_message_number, 0, sync);
        if (status != 1) {
            rtcm = osr2rtcm_end(rtcm);
        }
    }

    return rtcm;
}

// generate rtcm3 MSM 7
rtcm_t* gen_rtcm_msm7(OSR_GNSS* gnss, CommonObservation common, int sync) {
    if (gnss->satellite_count <= 0)
        return NULL;

    rtcm_t* rtcm = osr2rtcm_begin();
    if (rtcm != NULL) {
        rtcm->staid = common.rtcm_id;
        rtcm->time  = gtime_t{.time = gnss->time.time, .sec = gnss->time.sec};
        rtcm->clock_steering   = OptInteger{(int)common.clock_steering, true};
        rtcm->external_clock   = OptInteger{(int)common.external_clock, true};
        rtcm->smooth_indicator = OptInteger{(int)common.smooth_indicator, true};
        rtcm->smooth_interval  = OptInteger{(int)common.smooth_interval, true};
        osr2rtklib_convert(rtcm, gnss->system, gnss->satellites,
                           gnss->satellite_count);

        int rtcm_message_number =
            get_rtcm_message_number_for_gnss_system(gnss->system, 7);
        if (rtcm_message_number == -1) {
            rtcm = osr2rtcm_end(rtcm);
            return rtcm;
        }

        int status = gen_rtcm3(rtcm, rtcm_message_number, 0, sync);
        if (status != 1) {
            rtcm = osr2rtcm_end(rtcm);
        }
    }

    return rtcm;
}

// generate rtcm3 1230 message
rtcm_t* gen_rtcm_1230(RTK_GLO_BiasInformation bias) {
    rtcm_t* rtcm = osr2rtcm_begin();

    if (rtcm != NULL) {
        rtcm->staid            = bias.reference_station_id;
        rtcm->sta.glo_cp_align = bias.indicator;

        rtcm->sta.glo_cp_bias[0] = bias.l1_ca;
        rtcm->sta.glo_cp_bias[1] = bias.l1_p;
        rtcm->sta.glo_cp_bias[2] = bias.l2_ca;
        rtcm->sta.glo_cp_bias[3] = bias.l2_p;

        int status = gen_rtcm3(rtcm, 1230, 0, 0);
        if (status != 1) {
            rtcm = osr2rtcm_end(rtcm);
        }
    }

    return rtcm;
}