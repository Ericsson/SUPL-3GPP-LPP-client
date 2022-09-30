#pragma once
#include <utility/gps_time.h>
#include <utility/optional.h>
#include <utility/signal_id.h>

#define OSR_MAX_SIGNALS 64
#define OSR_MAX_SATELLITES 64
#define OSR_MAX_REFERENCE_STATIONS 64
#define RTK_MAX_RESIDUALS 32  // see RTCM spec 10403.3 DF006

struct RTK_Signal {
    SignalID         id;
    Optional<double> fine_PseudoRange;        // 1 ms  [+-(2^-10 - 2^29)]
    Optional<double> fine_PhaseRange;         // 1 ms  [+-(2^-8 - 2^31)]
    Optional<double> carrier_to_noise_ratio;  // 1 Hz  [0.0625 - 63.9375]
    Optional<double> fine_PhaseRangeRate;     // 1 m/s [+-1.6383]
    Optional<double> lockTimeIndicator;       // 1 sec
    Optional<bool>   halfCycleAmbiguityIndicator;
};

struct RTK_Satellite {
    long id;      // 0-63
    long system;  // SYS_GPS, SYS_GLO, SYS_GAL, SYS_CMP

    Optional<double> integer_ms;              // 1 ms  [0 - 254]
    Optional<double> rough_range;             // 1 ms  [0 - (1-2^-10)]
    Optional<double> rough_phase_range_rate;  // 1 m/s [+-8191]

    Optional<int> glo_frequency_channel;

    RTK_Signal* signals[OSR_MAX_SIGNALS];
    int         signal_count;
};

struct RTK_GLO_BiasInformation {
    long   reference_station_id;
    int    mask;
    int    indicator;
    double l1_ca, l1_p;
    double l2_ca, l2_p;
};

struct RTK_PhysicalReferenceStation {
    long   id;
    double x;
    double y;
    double z;
};

struct RTK_ReferenceStation {
    long        id;
    long        non_physical_ref_sta_id;
    bool        reference_station_is_physical;
    const char* name;

    double                                 x;
    double                                 y;
    double                                 z;
    Optional<double>                       antenna_height;
    Optional<RTK_PhysicalReferenceStation> physical;
};

struct RTK_Residuals_Element {
    long   satellite_id;
    double soc;  // 1 mm
    double sod;  // 1 ppm
    double soh;  // 1 ppm
    double slc;  // 1 mm
    double sld;  // 1 ppm
};

struct RTK_Residuals {
    long     system;  // SYS_GPS, SYS_GLO, SYS_GAL, SYS_CMP
    GPS_Time time;
    long     reference_station_id;
    long     n_refs;

    RTK_Residuals_Element list[RTK_MAX_RESIDUALS];
    long                  residual_elements_count;
};

struct OSR_GNSS {
    int      system;
    GPS_Time time;

    Optional<RTK_Residuals>           residuals;
    Optional<RTK_GLO_BiasInformation> bias_information;

    int            satellite_count;
    RTK_Satellite* satellites[OSR_MAX_SATELLITES];
};

struct CommonObservation {
    long rtcm_id;
    long clock_steering;
    long external_clock;
    long smooth_indicator;
    long smooth_interval;
};

struct OSR {
    Optional<CommonObservation>    common_observation;
    Optional<RTK_ReferenceStation> reference_station;

    OSR_GNSS gps;
    OSR_GNSS glo;
    OSR_GNSS gal;
    OSR_GNSS bds;

    RTK_Satellite* free_satellites;
    RTK_Signal*    free_signals;
    int            free_satellite_count;
    int            free_signal_count;
    int            free_satellite_capacity;
    int            free_signal_capacity;

    bool            last_update_had_reference_station;
    u32             system_mask;
    pthread_mutex_t lock;
};

void osr_init(OSR* od, int system_mask);
void osr_cleanup(OSR* osr);
void osr_reset(OSR* osr);

void _osr_lock(OSR* od, const char* file, int line, const char* func);
void _osr_unlock(OSR* od, const char* file, int line, const char* func);

RTK_Satellite* osr_allocate_satellite(OSR* osr);
RTK_Signal*    osr_allocate_signal(OSR* osr);
RTK_Satellite* osr_get_satellite(OSR_GNSS* gnss, long id);
RTK_Signal*    osr_get_signal(RTK_Satellite* satellite, SignalID id);
RTK_Satellite* osr_get_or_allocate_satellite(OSR* osr, OSR_GNSS* gnss, long id);
RTK_Signal*    osr_get_or_allocate_signal(OSR* osr, RTK_Satellite* satellite, SignalID id);

#define osr_lock(A) _osr_lock((A), __FILE__, __LINE__, __func__)
#define osr_unlock(A) _osr_unlock((A), __FILE__, __LINE__, __func__)

RTK_Satellite* osr_get_satellite(OSR_GNSS* gnss, long id);
