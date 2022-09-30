#include "osr/osr.h"

#include <rtklib.h>

void osr_init(OSR* od, int system_mask) {
    *od = {};
    pthread_mutex_init(&od->lock, NULL);

    od->system_mask = system_mask;
    od->gps.system  = SYS_GPS;
    od->glo.system  = SYS_GLO;
    od->gal.system  = SYS_GAL;
    od->bds.system  = SYS_CMP;

    od->free_satellite_capacity = 4 * OSR_MAX_REFERENCE_STATIONS * OSR_MAX_SATELLITES;
    od->free_signal_capacity    = 4 * OSR_MAX_REFERENCE_STATIONS * OSR_MAX_SATELLITES * OSR_MAX_SIGNALS;
    od->free_satellites =
        (RTK_Satellite*)malloc(sizeof(RTK_Satellite) * od->free_satellite_capacity);
    od->free_signals = (RTK_Signal*)malloc(sizeof(RTK_Signal) * od->free_signal_capacity);

    osr_reset(od);
}

void osr_cleanup(OSR* osr) {
    free(osr->free_satellites);
    free(osr->free_signals);
}

void osr_reset(OSR* osr) {
    osr->gps.satellite_count = 0;
    osr->glo.satellite_count = 0;
    osr->gal.satellite_count = 0;
    osr->bds.satellite_count = 0;

    osr->free_satellite_count              = osr->free_satellite_capacity;
    osr->free_signal_count                 = osr->free_signal_capacity;
    osr->last_update_had_reference_station = false;
}

void _osr_lock(OSR* od, const char* file, int line, const char* func) {
    pthread_mutex_lock(&od->lock);
}

void _osr_unlock(OSR* od, const char* file, int line, const char* func) {
    pthread_mutex_unlock(&od->lock);
}

RTK_Satellite* osr_allocate_satellite(OSR* osr) {
    assert(osr->free_satellite_count > 0);
    return &osr->free_satellites[--osr->free_satellite_count];
}

RTK_Signal* osr_allocate_signal(OSR* osr) {
    assert(osr->free_signal_count > 0);
    return &osr->free_signals[--osr->free_signal_count];
}

RTK_Satellite* osr_get_satellite(OSR_GNSS* gnss, long id) {
    for (auto i = 0; i < gnss->satellite_count; i++) {
        if (gnss->satellites[i] && gnss->satellites[i]->id == id) {
            return gnss->satellites[i];
        }
    }

    return NULL;
}

RTK_Signal* osr_get_signal(RTK_Satellite* satellite, SignalID id) {
    for (auto i = 0; i < satellite->signal_count; i++) {
        if (satellite->signals[i] && satellite->signals[i]->id == id) {
            return satellite->signals[i];
        }
    }

    return NULL;
}

RTK_Satellite* osr_get_or_allocate_satellite(OSR* osr, OSR_GNSS* gnss, long id) {
    auto satellite = osr_get_satellite(gnss, id);
    if (satellite) return satellite;
    return osr_allocate_satellite(osr);
}

RTK_Signal* osr_get_or_allocate_signal(OSR* osr, RTK_Satellite* satellite, SignalID id) {
    auto signal = osr_get_signal(satellite, id);
    if (signal) return signal;
    return osr_allocate_signal(osr);
}
