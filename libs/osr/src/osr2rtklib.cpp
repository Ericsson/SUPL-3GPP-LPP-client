//
//  ___  __     __   __   __   __
// |__  |__) | /  ` /__` /__` /  \ |\ |
// |___ |  \ | \__, .__/ .__/ \__/ | \|
//
//

#include "osr/osr2rtklib.h"
#include "osr/osr.h"
#if 0
// Convert satellite/signal data (OSR) to rtk_obs expected by RTKLIB
// TODO: Cleanup and document

static int loss_of_lock(OSR* osr, int sat, int freq, int lock) {
    int lli = (!lock && !osr->lockt[sat - 1][freq]) ||
              lock < osr->lockt[sat - 1][freq];
    osr->lockt[sat - 1][freq] = (unsigned short)lock;
    return lli;
}

int obsindex(obs_t* obs, gtime_t time, int sat) {
    int i, j;

    for (i = 0; i < obs->n; i++) {
        if (obs->data[i].sat == sat)
            return i; /* field already exists */
    }
    if (i >= MAXOBS)
        return -1; /* overflow */

    /* add new field */
    obs->data[i].time = time;
    char buffer[64];
#if 0
    time2str(time, buffer, 64);
    printf("\r#% 4i time: %s\n", i, buffer);
#endif
    obs->data[i].sat = sat;
    /*Initial values are set to 0*/
    for (j = 0; j < NFREQ + NEXOBS; j++) {
        obs->data[i].L[j] = obs->data[i].P[j] = 0.0;
        obs->data[i].D[j]                     = 0.0;
        obs->data[i].SNR[j] = obs->data[i].LLI[j] = obs->data[i].code[j] = 0;
    }
    obs->n++;
    return i;
}
#endif

constexpr static double RANGE_MS = (CLIGHT / 1000.0);

static void get_signal_index(int sys, const unsigned char* code, const int* freq, int n,
                             const char* opt, int* ind) {
    int i, nex, pri, pri_h[8] = {0}, index[8] = {0}, ex[32] = {0};

    /* test code priority */
    for (i = 0; i < n; i++) {
        if (!code[i]) continue;

        if (freq[i] > NFREQ) { /* save as extended signal if freq > NFREQ */
            ex[i] = 1;
            continue;
        }
        /* code priority */
        pri = getcodepri(sys, code[i], opt);

        /* select highest priority signal */
        if (pri > pri_h[freq[i] - 1]) {
            if (index[freq[i] - 1]) ex[index[freq[i] - 1] - 1] = 1;
            pri_h[freq[i] - 1] = pri;
            index[freq[i] - 1] = i + 1;
        } else
            ex[i] = 1;
    }
    /* signal index in obs data */
    for (i = nex = 0; i < n; i++) {
        if (ex[i] == 0)
            ind[i] = freq[i] - 1;
        else if (nex < NEXOBS)
            ind[i] = NFREQ + nex++;
        else { /* no space in obs data */
            trace(2, "rtcm msm: no space in obs data sys=%d code=%d\n", sys, code[i]);
            ind[i] = -1;
        }
#if 0
        trace(2,"sig pos: sys=%d code=%d ex=%d ind=%d\n",sys,code[i],ex[i],ind[i]);
#endif
    }
}

static const char* signal_name_from(int system, int rtcm_id) {
    switch (system) {
    case SYS_GPS: return msm_sig_gps[rtcm_id - 1];
    case SYS_GLO: return msm_sig_glo[rtcm_id - 1];
    case SYS_GAL: return msm_sig_gal[rtcm_id - 1];
    case SYS_QZS: return msm_sig_qzs[rtcm_id - 1];
    case SYS_SBS: return msm_sig_sbs[rtcm_id - 1];
    case SYS_CMP: return msm_sig_cmp[rtcm_id - 1];
    }

    return NULL;
}

static void sigindex(int sys, const uint8_t* code, int n, const char* opt, int* idx) {
    int i, nex, pri, pri_h[8] = {0}, index[8] = {0}, ex[32] = {0};

    /* test code priority */
    for (i = 0; i < n; i++) {
        if (!code[i]) continue;

        if (idx[i] >= NFREQ) { /* save as extended signal if idx >= NFREQ */
            ex[i] = 1;
            continue;
        }
        /* code priority */
        pri = getcodepri(sys, code[i], opt);

        /* select highest priority signal */
        if (pri > pri_h[idx[i]]) {
            if (index[idx[i]]) ex[index[idx[i]] - 1] = 1;
            pri_h[idx[i]] = pri;
            index[idx[i]] = i + 1;
        } else
            ex[i] = 1;
    }
    /* signal index in obs data */
    for (i = nex = 0; i < n; i++) {
        if (ex[i] == 0)
            ;
        else if (nex < NEXOBS)
            idx[i] = NFREQ + nex++;
        else { /* no space in obs data */
            trace(2, "rtcm msm: no space in obs data sys=%d code=%d\n", sys, code[i]);
            idx[i] = -1;
        }
#if 0 /* for debug */
        printf("sig pos: sys=%d code=%d ex=%d idx=%d\n",sys,code[i],ex[i],idx[i]);
#endif
    }
}

static int lossoflock(rtcm_t* rtcm, int sat, int freq, int lock) {
    int lli = (!lock && !rtcm->lock[sat - 1][freq]) || lock < rtcm->lock[sat - 1][freq];
    rtcm->lock[sat - 1][freq] = (unsigned short)lock;
    return lli;
}

static int obsindex(obs_t* obs, gtime_t time, int sat) {
    int i, j;

    for (i = 0; i < obs->n; i++) {
        if (obs->data[i].sat == sat) return i; /* field already exists */
    }
    if (i >= MAXOBS) return -1; /* overflow */

    /* add new field */
    obs->data[i].time = time;
    obs->data[i].sat  = sat;
    for (j = 0; j < NFREQ + NEXOBS; j++) {
        obs->data[i].L[j] = obs->data[i].P[j] = 0.0;
        obs->data[i].D[j]                     = 0.0;
        obs->data[i].SNR[j] = obs->data[i].LLI[j] = obs->data[i].code[j] = 0;
    }
    obs->n++;
    return i;
}

static int rtcm3_satellite_idx(int system, int rtcm_id) {
    if (system == SYS_CMP)
        rtcm_id += MINPRNCMP - 1;
    else if (system == SYS_GPS)
        rtcm_id += MINPRNGPS - 1;
    else if (system == SYS_GAL)
        rtcm_id += MINPRNGAL - 1;
    else if (system == SYS_GLO)
        rtcm_id += MINPRNGLO - 1;

    auto sat = satno(system, rtcm_id);
    if (sat <= 0) return -1;
    return sat;
}

static int rtcm3_satellite_id(int system, int lpp_id) {
    return lpp_id + 1;
}

static int to_sigid(int sys, uint8_t code) {
    const char* const* msm_sig;
    char*              sig;
    int                i;

    /* signal conversion for undefined signal by rtcm */
    if (sys == SYS_GPS) {
        if (code == CODE_L1Y)
            code = CODE_L1P;
        else if (code == CODE_L1M)
            code = CODE_L1P;
        else if (code == CODE_L1N)
            code = CODE_L1P;
        else if (code == CODE_L2D)
            code = CODE_L2P;
        else if (code == CODE_L2Y)
            code = CODE_L2P;
        else if (code == CODE_L2M)
            code = CODE_L2P;
        else if (code == CODE_L2N)
            code = CODE_L2P;
    }
    if (!*(sig = code2obs(code))) return 0;

    switch (sys) {
    case SYS_GPS: msm_sig = msm_sig_gps; break;
    case SYS_GLO: msm_sig = msm_sig_glo; break;
    case SYS_GAL: msm_sig = msm_sig_gal; break;
    case SYS_QZS: msm_sig = msm_sig_qzs; break;
    case SYS_SBS: msm_sig = msm_sig_sbs; break;
    case SYS_CMP: msm_sig = msm_sig_cmp; break;
    default: return 0;
    }
    for (i = 0; i < 32; i++) {
        if (!strcmp(sig, msm_sig[i])) return i + 1;
    }
    return 0;
}

void osr2rtklib_convert(rtcm_t* rtcm, int system, RTK_Satellite** satellites, int satellite_count) {
    const char*   signal_names[64];
    unsigned char signal_code[64];
    int           signal_idx[64];

    for (auto i = 0; i < satellite_count; i++) {
        auto sat         = satellites[i];
        auto sat_rtcm_id = rtcm3_satellite_id(system, sat->id);
        auto sat_idx     = rtcm3_satellite_idx(system, sat_rtcm_id);
        if (sat_idx < 0) {
            continue;
        }

        auto obs_idx = obsindex(&rtcm->obs, rtcm->time, sat_idx);
        if (obs_idx < 0) {
            continue;
        }

        for (auto j = 0; j < sat->signal_count; j++) {
            signal_idx[j] = -1;
        }

        for (auto j = 0; j < sat->signal_count; j++) {
            auto sig     = sat->signals[j];
            auto rtcm_id = sig->id.rtcm_msm_id();
            if (!rtcm_id.initialized()) {
                continue;
            }

            auto sig_name = signal_name_from(system, rtcm_id.value());
            if (!sig_name) {
                continue;
            }

            auto name      = std::string{sig->id.name()};
            signal_code[j] = obs2code(sig_name);
            signal_idx[j]  = code2idx(system, signal_code[j]);
        }

        sigindex(system, signal_code, sat->signal_count, rtcm->opt, signal_idx);

        Optional<double> range;
        if (sat->integer_ms.initialized()) {
            auto value = sat->integer_ms.value();
            if (sat->rough_range.initialized()) {
                auto rr = sat->rough_range.value();
                value += rr;
            }

            range = value;
        }

        auto fcn = 0;
        if (system == SYS_GLO) {
            if (sat->glo_frequency_channel.initialized()) {
                fcn = sat->glo_frequency_channel.value();
            } else {
                fcn = -8; /* no glonass fcn info */
            }
        }

        auto obsd = &rtcm->obs.data[obs_idx];
        obsd->sat = sat_idx;
        for (auto j = 0; j < NFREQ + NEXOBS; j++) {
            obsd->code[j] = CODE_NONE;
        }

        auto code_index = 0;
        for (auto j = 0; j < sat->signal_count; j++) {
            auto sig = sat->signals[j];
            if (signal_code[j] < 0) {
                auto sv = sig->id.name();
                continue;
            }

            auto sv   = sig->id.name();
            auto idx  = code_index++;
            auto code = signal_code[j];
            if (code_index > NFREQ + NEXOBS) continue;
            auto freq = 0.0;
            if (fcn >= -7) {
                freq = code2freq(system, code, fcn);
            }

            /* pseudorange (m) */
            if (range.initialized() && sig->fine_PseudoRange.initialized()) {
                auto r  = range.value() * RANGE_MS;
                auto pr = sig->fine_PseudoRange.value() * RANGE_MS;
                if (r != 0.0 && pr > -1E12) {
                    obsd->P[idx] = r + pr;
                }
            }

            /* carrier-phase (cycle) */
            if (range.initialized() && sig->fine_PhaseRange.initialized()) {
                auto r  = range.value() * RANGE_MS;
                auto cp = sig->fine_PhaseRange.value();
                cp *= RANGE_MS;
                if (r != 0.0 && cp > -1E12) {
                    obsd->L[idx] = (r + cp) * freq / CLIGHT;
                }
            }

            /* doppler (hz) */
            if (sat->rough_phase_range_rate.initialized() &&
                sig->fine_PhaseRangeRate.initialized()) {
                auto rr  = sat->rough_phase_range_rate.value();
                auto rrf = sig->fine_PhaseRangeRate.value();
                if (rrf > -1E12) {
                    obsd->D[idx] = (float)(-(rr + rrf) * freq / CLIGHT);
                }
            }

            auto lock = sig->lockTimeIndicator.or_value(0);
            auto half = sig->halfCycleAmbiguityIndicator.or_value(0);
            auto cnr  = sig->carrier_to_noise_ratio.or_value(0);

            obsd->LLI[idx]  = lossoflock(rtcm, sat_idx, idx, lock) + (half ? 2 : 0);
            obsd->SNR[idx]  = (uint16_t)(cnr / SNR_UNIT + 0.5);
            obsd->code[idx] = code;

            {
                obsd->integer_ms[idx] = OptDouble{
                    .value     = sat->integer_ms.or_value(0.0),
                    .available = sat->integer_ms.initialized(),
                };

                obsd->rough_range_modulo_ms[idx] = OptDouble{
                    .value     = sat->rough_range.or_value(0.0),
                    .available = sat->rough_range.initialized(),
                };

                obsd->rough_phase_range_rates[idx] = OptDouble{
                    .value     = sat->rough_phase_range_rate.or_value(0.0),
                    .available = sat->rough_phase_range_rate.initialized(),
                };

                if (sat->glo_frequency_channel.initialized()) {
                    obsd->extended_satellite_info[idx] = sat->glo_frequency_channel.value() + 7;
                } else {
                    obsd->extended_satellite_info[idx] = 15;
                }

                obsd->fine_pseudo_range[idx] = OptDouble{
                    .value     = sig->fine_PseudoRange.or_value(0.0),
                    .available = sig->fine_PseudoRange.initialized(),
                };

                obsd->fine_phase_range[idx] = OptDouble{
                    .value     = sig->fine_PhaseRange.or_value(0.0),
                    .available = sig->fine_PhaseRange.initialized(),
                };

                obsd->fine_phase_range_rates[idx] = OptDouble{
                    .value     = sig->fine_PhaseRangeRate.or_value(0.0),
                    .available = sig->fine_PhaseRangeRate.initialized(),
                };

                obsd->carrier_to_noise_ratio[idx] = OptDouble{
                    .value     = sig->carrier_to_noise_ratio.or_value(0.0),
                    .available = sig->carrier_to_noise_ratio.initialized(),
                };

                obsd->phase_range_lock[idx] = OptDouble{
                    .value     = sig->lockTimeIndicator.or_value(0.0),
                    .available = sig->lockTimeIndicator.initialized(),
                };

                obsd->half_cycle[idx] = sig->halfCycleAmbiguityIndicator.or_value(0);
            }
        }
    }
}

#if 0
void osr2rtklib_convert(obs_t* dst, nav_t* nav, OSR* osr, int sys, gtime_t time,
                        RTK_Satellite* satellites, int satellite_count) {
    int i = 0, j = 0, k = 0, n = 0;
    int sat = 0, satellite_index = 0;

    for (i = 0; i < satellite_count; i++) {
        RTKSatellite* satellite = &satellites[i];
        // satellite->id is 0 indexed
        // But RTKLIB has satellites indexed from 1
        int sat_id = satellite->id + 1;
        if (sys == SYS_QZS)
            sat_id += MINPRNQZS - 1;
        else if (sys == SYS_SBS)
            sat_id += MINPRNSBS - 1;
        if ((sat = satno(sys, sat_id))) {
            // Get/add index in obsdata for satellite
            satellite_index = obsindex(dst, time, sat);
        }

        if (sat <= 0 || satellite_index < 0)
            continue;

        obsd_t* dst_satellite = &dst->data[satellite_index];
        for (k = 0; k < satellite->signal_count; k++) {
            RTKSignal* signal = &satellite->signals[k];

            const char* signal_type = "";
            switch (sys) {
            case SYS_GPS:
                signal_type = msm_sig_gps[signal->id];
                break;
            case SYS_GLO:
                signal_type = msm_sig_glo[signal->id];
                break;
            case SYS_GAL:
                signal_type = msm_sig_gal[signal->id];
                break;
            case SYS_QZS:
                signal_type = msm_sig_qzs[signal->id];
                break;
            case SYS_SBS:
                signal_type = msm_sig_sbs[signal->id];
                break;
            case SYS_CMP:
                signal_type = msm_sig_cmp[signal->id];
                break;
            }

            // Convert signal_type ("1C", ...) to signal_code (CODE_?)
            int           signal_freq;
            unsigned char signal_code =
                obs2code(sys, signal_type, &signal_freq);
            // unsigned char signal_code =
            //     obs2code(signal_type);
            if (signal_freq < 0)
                continue;

            // Get signal indicies in dst->data, saved to ind
            // get_signal_index(sys, code, freq, satellite->signal_count, "",
            // ind);
            int signal_index;
            get_signal_index(sys, &signal_code, &signal_freq, 1, "",
                             &signal_index);

            // Satellite carrier wave length
            double wave_length = satwavelen(sat, signal_freq - 1, nav);

#if 0
                // glonass wave length by extended info
                // TODO(ewasjon): Where is the extended info saved for signals
                if (sys == SYS_GLO && ex && ex[i] <= 13) {
                    fn = ex[i] - 7;
                    wave_length =
                        CLIGHT / ((freq[k] == 2 ? FREQ2_GLO : FREQ1_GLO) + (freq[k] == 2 ? DFRQ2_GLO : DFRQ1_GLO) * fn);
                }
#endif

            // Pseudorange (m)
            if (satellite->rough_range != 0.0 &&
                signal->fine_PseudoRange > -1E12) {
                dst_satellite->P[signal_index] = satellite->rough_range +
                                                 satellite->integer_ms +
                                                 signal->fine_PseudoRange;
            }

            // Carrier-phase (cycle)
            if (satellite->rough_range != 0.0 &&
                signal->fine_PhaseRange > -1E12 && wave_length > 0.0) {
                dst_satellite->L[signal_index] =
                    (satellite->rough_range + satellite->integer_ms +
                     signal->fine_PhaseRange) /
                    wave_length;
            }

            // Doppler (hz)
            if (signal->fine_PhaseRangeRate > -1E12 && wave_length > 0.0) {
                dst_satellite->D[signal_index] =
                    (float)(-(satellite->rough_phase_range_rate +
                              signal->fine_PhaseRangeRate) /
                            wave_length);
            }

            // TODO(ewasjon): Is this correct?
            dst_satellite->LLI[signal_index] =
                loss_of_lock(osr, sat, signal_index,
                             signal->lockTimeIndicator) +
                (signal->halfCycleAmbiguityIndicator ? 3 : 0);
            dst_satellite->SNR[signal_index] =
                (unsigned char)(signal->carrier_to_noise_ratio * 4.0);
            dst_satellite->code[signal_index] = signal_code;
        }
    }
}

// extern double        satwavelen(int sat, int frq, const nav_t* nav);
// extern int           satno(int sys, int prn);
// extern unsigned char obs2code(int sys, const char* obs, int* freq);
#endif