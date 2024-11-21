#include "observation.hpp"
#include "data.hpp"
#include "geoid.hpp"
#include "helper.hpp"
#include "mops.hpp"
#include "satellite.hpp"

#include <loglet/loglet.hpp>
#include <maths/mat3.hpp>
#include <time/utc.hpp>

#define LOGLET_CURRENT_MODULE "tokoro"

namespace generator {
namespace tokoro {

Observation::Observation(Satellite const& satellite, SignalId signal_id, Float3 location) NOEXCEPT
    : mSvId(satellite.id()),
      mSignalId(signal_id),
      mTrueRange(satellite.true_range()),
      mEphRange(satellite.eph_range()),
      mCodeBiasValid(false),
      mPhaseBiasValid(false),
      mTropoValid(false),
      mIonoValid(false),
      mShapiroValid(false),
      mPhaseWindupValid(false),
      mSolidTidesValid(false) {
    // TODO(ewasjon): For GLONASS, the frequency depends on the channel number
    mFrequency  = signal_id.frequency();
    mWavelength = constant::SPEED_OF_LIGHT / mFrequency;

    mEphOrbitError = mEphRange - mTrueRange;

    // NOTE(ewasjon): The clock bias should be added to the satellite time to "correct" it, however,
    // we want to uncorrect the satellite time, so we subtract it.
    mEphClockBias = -satellite.eph_clock_bias();

    mLineOfSight = satellite.line_of_sight();

    mClockCorrection      = satellite.clock_correction();
    mClockCorrectionValid = true;

    mEmissionTime   = satellite.emission_time();
    mReceptionTime  = satellite.reception_time();
    mGroundPosition = location;
    mWgsPosition    = ecef_to_wgs84(location);
    mElevation      = satellite.elevation();

    mSatelliteApc = satellite.apc();

    auto mapping     = hydrostatic_mapping_function(mReceptionTime, mWgsPosition, mElevation);
    mTropoDryMapping = mapping.hydrostatic;
    mTropoWetMapping = mapping.wet;
}

void Observation::compute_tropospheric_height() NOEXCEPT {
    VSCOPE_FUNCTION();

    auto ellipsoidal_height = mWgsPosition.z;
    auto geoid_height       = Geoid::height(mWgsPosition.x, mWgsPosition.y, Geoid::Model::EMBEDDED);

    HydrostaticAndWetDelay alt_0{};
    HydrostaticAndWetDelay alt_eh{};
    auto mops_0 = mops_tropospheric_delay(mReceptionTime, mWgsPosition.x, 0.0, geoid_height, alt_0);
    auto mops_eh = mops_tropospheric_delay(mReceptionTime, mWgsPosition.x, ellipsoidal_height,
                                           geoid_height, alt_eh);
    if (mops_0 && mops_eh) {
        mTropoDryHeightCorrection   = alt_eh.hydrostatic / alt_0.hydrostatic;
        mTropoWetHeightCorrection   = alt_eh.wet / alt_0.wet;
        mTropeHeightCorrectionValid = true;
    } else {
        WARNF("failed to compute tropospheric height correction");
        mTropeHeightCorrectionValid = false;
    }
}

void Observation::compute_phase_bias(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    auto signals = correction_data.signal_corrections(mSvId);
    if (!signals) return;

    auto it = signals->phase_bias.find(mSignalId);
    if (it == signals->phase_bias.end()) return;

    mPhaseBias      = it->second.bias;
    mPhaseBiasValid = true;
}

void Observation::compute_code_bias(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    auto signals = correction_data.signal_corrections(mSvId);
    if (!signals) return;

    auto it = signals->code_bias.find(mSignalId);
    if (it == signals->code_bias.end()) return;

    mCodeBias      = it->second.bias;
    mCodeBiasValid = true;
}

void Observation::compute_tropospheric(EcefPosition          location,
                                       CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mSvId.name());

    VERBOSEF("position: (%f, %f, %f)", location.x, location.y, location.z);

    TroposphericCorrection correction{};
    if (!correction_data.tropospheric(mSvId, location, correction)) {
        VERBOSEF("tropospheric correction not found");
        return;
    }

    if (mTropoValid) {
        VERBOSEF("tropospheric correction already computed");
        return;
    }

    mTropoDry   = correction.dry;
    mTropoWet   = correction.wet;
    mTropoValid = true;
}

void Observation::compute_ionospheric(EcefPosition          location,
                                      CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    IonosphericCorrection correction{};
    if (!correction_data.ionospheric(mSvId, location, correction)) {
        VERBOSEF("ionospheric correction not found");
        return;
    }

    mIonoGridResidual = correction.grid_residual;
    mIonoPolyResidual = correction.polynomial_residual;
    mIonoValid        = true;
}

void Observation::compute_shapiro() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    auto r_sat = geocentric_distance(mSatelliteApc);
    auto r_rcv = geocentric_distance(mGroundPosition);
    auto r     = mTrueRange;

    // https://gssc.esa.int/navipedia/index.php/Relativistic_Path_Range_Effect
    auto shapiro = (2 * constant::GME / (constant::SPEED_OF_LIGHT * constant::SPEED_OF_LIGHT)) *
                   log((r_sat + r_rcv + r) / (r_sat + r_rcv - r));

    mShapiro      = shapiro;
    mShapiroValid = true;
}

void Observation::compute_phase_windup() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
}

struct AstronomicalArguments {
    double l;      // mean anomaly of the Moon
    double lp;     // mean anomaly of the Sun
    double f;      // mean argument of the latitude of the Moon
    double d;      // mean elongation of the Moon from the Sun
    double omega;  // mean longitude of the ascending node of the Moon
};

static double const ASTRONOMICAL_ARGUMENTS_DATA[][5] = {
    /* coefficients for iau 1980 nutation */
    {134.96340251, 1717915923.2178, 31.8792, 0.051635, -0.00024470},
    {357.52910918, 129596581.0481, -0.5532, 0.000136, -0.00001149},
    {93.27209062, 1739527262.8478, -12.7512, -0.001037, 0.00000417},
    {297.85019547, 1602961601.2090, -6.3706, 0.006593, -0.00003169},
    {125.04455501, -6962890.2665, 7.4722, 0.007702, -0.00005939},
};

static bool compute_astronomical_arguments(double t_jc, AstronomicalArguments& args) NOEXCEPT {
    VSCOPE_FUNCTION();

    double time[4];
    time[0] = t_jc;
    time[1] = time[0] * t_jc;
    time[2] = time[1] * t_jc;
    time[3] = time[2] * t_jc;

    args.l     = ASTRONOMICAL_ARGUMENTS_DATA[0][0] * 3600.0;
    args.lp    = ASTRONOMICAL_ARGUMENTS_DATA[1][0] * 3600.0;
    args.f     = ASTRONOMICAL_ARGUMENTS_DATA[2][0] * 3600.0;
    args.d     = ASTRONOMICAL_ARGUMENTS_DATA[3][0] * 3600.0;
    args.omega = ASTRONOMICAL_ARGUMENTS_DATA[4][0] * 3600.0;

    for (int i = 0; i < 4; i++) {
        args.l += ASTRONOMICAL_ARGUMENTS_DATA[0][i + 1] * time[i];
        args.lp += ASTRONOMICAL_ARGUMENTS_DATA[1][i + 1] * time[i];
        args.f += ASTRONOMICAL_ARGUMENTS_DATA[2][i + 1] * time[i];
        args.d += ASTRONOMICAL_ARGUMENTS_DATA[3][i + 1] * time[i];
        args.omega += ASTRONOMICAL_ARGUMENTS_DATA[4][i + 1] * time[i];
    }

    args.l     = fmod(args.l * constant::ARCSEC2RAD, 2 * constant::PI);
    args.lp    = fmod(args.lp * constant::ARCSEC2RAD, 2 * constant::PI);
    args.f     = fmod(args.f * constant::ARCSEC2RAD, 2 * constant::PI);
    args.d     = fmod(args.d * constant::ARCSEC2RAD, 2 * constant::PI);
    args.omega = fmod(args.omega * constant::ARCSEC2RAD, 2 * constant::PI);
    return true;
}

struct Iau1980Nutation {
    double d_psi;  // nutation in longitude
    double d_eps;  // nutation in obliquity
};

static double const IAU1980_NUTATION_DATA[106][10] = {
    {0, 0, 0, 0, 1, -6798.4, -171996, -174.2, 92025, 8.9},
    {0, 0, 2, -2, 2, 182.6, -13187, -1.6, 5736, -3.1},
    {0, 0, 2, 0, 2, 13.7, -2274, -0.2, 977, -0.5},
    {0, 0, 0, 0, 2, -3399.2, 2062, 0.2, -895, 0.5},
    {0, -1, 0, 0, 0, -365.3, -1426, 3.4, 54, -0.1},
    {1, 0, 0, 0, 0, 27.6, 712, 0.1, -7, 0.0},
    {0, 1, 2, -2, 2, 121.7, -517, 1.2, 224, -0.6},
    {0, 0, 2, 0, 1, 13.6, -386, -0.4, 200, 0.0},
    {1, 0, 2, 0, 2, 9.1, -301, 0.0, 129, -0.1},
    {0, -1, 2, -2, 2, 365.2, 217, -0.5, -95, 0.3},
    {-1, 0, 0, 2, 0, 31.8, 158, 0.0, -1, 0.0},
    {0, 0, 2, -2, 1, 177.8, 129, 0.1, -70, 0.0},
    {-1, 0, 2, 0, 2, 27.1, 123, 0.0, -53, 0.0},
    {1, 0, 0, 0, 1, 27.7, 63, 0.1, -33, 0.0},
    {0, 0, 0, 2, 0, 14.8, 63, 0.0, -2, 0.0},
    {-1, 0, 2, 2, 2, 9.6, -59, 0.0, 26, 0.0},
    {-1, 0, 0, 0, 1, -27.4, -58, -0.1, 32, 0.0},
    {1, 0, 2, 0, 1, 9.1, -51, 0.0, 27, 0.0},
    {-2, 0, 0, 2, 0, -205.9, -48, 0.0, 1, 0.0},
    {-2, 0, 2, 0, 1, 1305.5, 46, 0.0, -24, 0.0},
    {0, 0, 2, 2, 2, 7.1, -38, 0.0, 16, 0.0},
    {2, 0, 2, 0, 2, 6.9, -31, 0.0, 13, 0.0},
    {2, 0, 0, 0, 0, 13.8, 29, 0.0, -1, 0.0},
    {1, 0, 2, -2, 2, 23.9, 29, 0.0, -12, 0.0},
    {0, 0, 2, 0, 0, 13.6, 26, 0.0, -1, 0.0},
    {0, 0, 2, -2, 0, 173.3, -22, 0.0, 0, 0.0},
    {-1, 0, 2, 0, 1, 27.0, 21, 0.0, -10, 0.0},
    {0, 2, 0, 0, 0, 182.6, 17, -0.1, 0, 0.0},
    {0, 2, 2, -2, 2, 91.3, -16, 0.1, 7, 0.0},
    {-1, 0, 0, 2, 1, 32.0, 16, 0.0, -8, 0.0},
    {0, 1, 0, 0, 1, 386.0, -15, 0.0, 9, 0.0},
    {1, 0, 0, -2, 1, -31.7, -13, 0.0, 7, 0.0},
    {0, -1, 0, 0, 1, -346.6, -12, 0.0, 6, 0.0},
    {2, 0, -2, 0, 0, -1095.2, 11, 0.0, 0, 0.0},
    {-1, 0, 2, 2, 1, 9.5, -10, 0.0, 5, 0.0},
    {1, 0, 2, 2, 2, 5.6, -8, 0.0, 3, 0.0},
    {0, -1, 2, 0, 2, 14.2, -7, 0.0, 3, 0.0},
    {0, 0, 2, 2, 1, 7.1, -7, 0.0, 3, 0.0},
    {1, 1, 0, -2, 0, -34.8, -7, 0.0, 0, 0.0},
    {0, 1, 2, 0, 2, 13.2, 7, 0.0, -3, 0.0},
    {-2, 0, 0, 2, 1, -199.8, -6, 0.0, 3, 0.0},
    {0, 0, 0, 2, 1, 14.8, -6, 0.0, 3, 0.0},
    {2, 0, 2, -2, 2, 12.8, 6, 0.0, -3, 0.0},
    {1, 0, 0, 2, 0, 9.6, 6, 0.0, 0, 0.0},
    {1, 0, 2, -2, 1, 23.9, 6, 0.0, -3, 0.0},
    {0, 0, 0, -2, 1, -14.7, -5, 0.0, 3, 0.0},
    {0, -1, 2, -2, 1, 346.6, -5, 0.0, 3, 0.0},
    {2, 0, 2, 0, 1, 6.9, -5, 0.0, 3, 0.0},
    {1, -1, 0, 0, 0, 29.8, 5, 0.0, 0, 0.0},
    {1, 0, 0, -1, 0, 411.8, -4, 0.0, 0, 0.0},
    {0, 0, 0, 1, 0, 29.5, -4, 0.0, 0, 0.0},
    {0, 1, 0, -2, 0, -15.4, -4, 0.0, 0, 0.0},
    {1, 0, -2, 0, 0, -26.9, 4, 0.0, 0, 0.0},
    {2, 0, 0, -2, 1, 212.3, 4, 0.0, -2, 0.0},
    {0, 1, 2, -2, 1, 119.6, 4, 0.0, -2, 0.0},
    {1, 1, 0, 0, 0, 25.6, -3, 0.0, 0, 0.0},
    {1, -1, 0, -1, 0, -3232.9, -3, 0.0, 0, 0.0},
    {-1, -1, 2, 2, 2, 9.8, -3, 0.0, 1, 0.0},
    {0, -1, 2, 2, 2, 7.2, -3, 0.0, 1, 0.0},
    {1, -1, 2, 0, 2, 9.4, -3, 0.0, 1, 0.0},
    {3, 0, 2, 0, 2, 5.5, -3, 0.0, 1, 0.0},
    {-2, 0, 2, 0, 2, 1615.7, -3, 0.0, 1, 0.0},
    {1, 0, 2, 0, 0, 9.1, 3, 0.0, 0, 0.0},
    {-1, 0, 2, 4, 2, 5.8, -2, 0.0, 1, 0.0},
    {1, 0, 0, 0, 2, 27.8, -2, 0.0, 1, 0.0},
    {-1, 0, 2, -2, 1, -32.6, -2, 0.0, 1, 0.0},
    {0, -2, 2, -2, 1, 6786.3, -2, 0.0, 1, 0.0},
    {-2, 0, 0, 0, 1, -13.7, -2, 0.0, 1, 0.0},
    {2, 0, 0, 0, 1, 13.8, 2, 0.0, -1, 0.0},
    {3, 0, 0, 0, 0, 9.2, 2, 0.0, 0, 0.0},
    {1, 1, 2, 0, 2, 8.9, 2, 0.0, -1, 0.0},
    {0, 0, 2, 1, 2, 9.3, 2, 0.0, -1, 0.0},
    {1, 0, 0, 2, 1, 9.6, -1, 0.0, 0, 0.0},
    {1, 0, 2, 2, 1, 5.6, -1, 0.0, 1, 0.0},
    {1, 1, 0, -2, 1, -34.7, -1, 0.0, 0, 0.0},
    {0, 1, 0, 2, 0, 14.2, -1, 0.0, 0, 0.0},
    {0, 1, 2, -2, 0, 117.5, -1, 0.0, 0, 0.0},
    {0, 1, -2, 2, 0, -329.8, -1, 0.0, 0, 0.0},
    {1, 0, -2, 2, 0, 23.8, -1, 0.0, 0, 0.0},
    {1, 0, -2, -2, 0, -9.5, -1, 0.0, 0, 0.0},
    {1, 0, 2, -2, 0, 32.8, -1, 0.0, 0, 0.0},
    {1, 0, 0, -4, 0, -10.1, -1, 0.0, 0, 0.0},
    {2, 0, 0, -4, 0, -15.9, -1, 0.0, 0, 0.0},
    {0, 0, 2, 4, 2, 4.8, -1, 0.0, 0, 0.0},
    {0, 0, 2, -1, 2, 25.4, -1, 0.0, 0, 0.0},
    {-2, 0, 2, 4, 2, 7.3, -1, 0.0, 1, 0.0},
    {2, 0, 2, 2, 2, 4.7, -1, 0.0, 0, 0.0},
    {0, -1, 2, 0, 1, 14.2, -1, 0.0, 0, 0.0},
    {0, 0, -2, 0, 1, -13.6, -1, 0.0, 0, 0.0},
    {0, 0, 4, -2, 2, 12.7, 1, 0.0, 0, 0.0},
    {0, 1, 0, 0, 2, 409.2, 1, 0.0, 0, 0.0},
    {1, 1, 2, -2, 2, 22.5, 1, 0.0, -1, 0.0},
    {3, 0, 2, -2, 2, 8.7, 1, 0.0, 0, 0.0},
    {-2, 0, 2, 2, 2, 14.6, 1, 0.0, -1, 0.0},
    {-1, 0, 0, 0, 2, -27.3, 1, 0.0, -1, 0.0},
    {0, 0, -2, 2, 1, -169.0, 1, 0.0, 0, 0.0},
    {0, 1, 2, 0, 1, 13.1, 1, 0.0, 0, 0.0},
    {-1, 0, 4, 0, 2, 9.1, 1, 0.0, 0, 0.0},
    {2, 1, 0, -2, 0, 131.7, 1, 0.0, 0, 0.0},
    {2, 0, 0, 2, 0, 7.1, 1, 0.0, 0, 0.0},
    {2, 0, 2, -2, 1, 12.8, 1, 0.0, -1, 0.0},
    {2, 0, -2, 0, 1, -943.2, 1, 0.0, 0, 0.0},
    {1, -1, 0, -2, 0, -29.3, 1, 0.0, 0, 0.0},
    {-1, 0, 0, 1, 1, -388.3, 1, 0.0, 0, 0.0},
    {-1, -1, 0, 2, 1, 35.0, 1, 0.0, 0, 0.0},
    {0, 1, 0, 1, 0, 27.3, 1, 0.0, 0, 0.0},
};

static bool compute_iau1980_nutation(double t, AstronomicalArguments const& args,
                                     Iau1980Nutation& nutation) {
    VSCOPE_FUNCTIONF("%f", t);

    nutation.d_psi = 0.0;
    nutation.d_eps = 0.0;

    for (auto i = 0; i < 106; i++) {
        auto angle = 0.0;
        angle += IAU1980_NUTATION_DATA[i][0] * args.l;
        angle += IAU1980_NUTATION_DATA[i][1] * args.lp;
        angle += IAU1980_NUTATION_DATA[i][2] * args.f;
        angle += IAU1980_NUTATION_DATA[i][3] * args.d;
        angle += IAU1980_NUTATION_DATA[i][4] * args.omega;

        nutation.d_psi +=
            (IAU1980_NUTATION_DATA[i][6] + IAU1980_NUTATION_DATA[i][7] * t) * sin(angle);
        nutation.d_eps +=
            (IAU1980_NUTATION_DATA[i][8] + IAU1980_NUTATION_DATA[i][9] * t) * cos(angle);
    }

    // 0.1 mas to rad
    nutation.d_psi *= 1.0e-4 * constant::ARCSEC2RAD;
    nutation.d_eps *= 1.0e-4 * constant::ARCSEC2RAD;
    return true;
}

static bool compute_sun_and_moon_position_eci(ts::Tai const& time, Float3& sun,
                                              Float3& moon) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    // Convert the time to UT1 (Universal Time 1)
    auto utc_time = ts::Utc{time};

    // TODO(ewasjon): Need UT1-UTC correction term, usually given by Earth Orientation Parameters
    // (EOP). However, UTC is by definition maintained to be within +-0.9s of UT1, so it probably
    // doesn't matter for our purposes.
    auto ut1_utc  = 0.0;
    auto ut1_time = utc_time.ut1(ut1_utc);

    // Compute seconds since J2000
    auto j2000 = ts::Utc::from_date_time(2000, 1, 1, 12, 0, 0);
    auto t_js  = (ut1_time - j2000.timestamp()).full_seconds();

    // Get the Julian centuries since J2000
    auto t_jc = t_js / 86400.0 / 36525.0;

    // Get astronomical arguments
    AstronomicalArguments args{};
    if (!compute_astronomical_arguments(t_jc, args)) {
        VERBOSEF("failed to compute astronomical arguments");
        return false;
    }

    // Obliquity of the ecliptic
    auto epsilon = 23.439291 - 0.000130042 * t_jc;
    auto sine    = sin(epsilon * constant::DEG2RAD);
    auto cose    = cos(epsilon * constant::DEG2RAD);

    {
        // Sun
        auto ms = 357.5277233 + 35999.05034 * t_jc;
        auto ls = 280.460 + 36000.770 * t_jc + 1.914666471 * sin(ms * constant::DEG2RAD) +
                  0.019994643 * sin(2.0 * ms * constant::DEG2RAD);
        auto rs   = constant::AU * (1.000140612 - 0.016708617 * cos(ms * constant::DEG2RAD) -
                                  0.000139589 * cos(2.0 * ms * constant::DEG2RAD));
        auto sinl = sin(ls * constant::DEG2RAD);
        auto cosl = cos(ls * constant::DEG2RAD);

        sun.x = rs * cosl;
        sun.y = rs * sinl * cose;
        sun.z = rs * sinl * sine;
    }

    {
        // Moon
        auto mm = 218.32 + 481267.883 * t_jc + 6.29 * sin(args.l) -
                  1.27 * sin(args.l - 2.0 * args.d) + 0.66 * sin(2.0 * args.d) +
                  0.21 * sin(2.0 * args.l) - 0.19 * sin(args.lp) - 0.11 * sin(2.0 * args.f);
        auto pm = 5.13 * sin(args.f) + 0.28 * sin(args.l + args.f) - 0.28 * sin(args.f - args.l) -
                  0.17 * sin(args.f - 2.0 * args.d);
        auto rm = constant::RE_WGS84 /
                  sin((0.9508 + 0.0518 * cos(args.l) + 0.0095 * cos(args.l - 2.0 * args.d) +
                       0.0078 * cos(2.0 * args.d) + 0.0028 * cos(2.0 * args.l)) *
                      constant::DEG2RAD);

        auto sinl = sin(pm * constant::DEG2RAD);
        auto cosl = cos(pm * constant::DEG2RAD);
        auto sinp = sin(mm * constant::DEG2RAD);
        auto cosp = cos(mm * constant::DEG2RAD);

        moon.x = rm * cosp * cosl;
        moon.y = rm * (cose * cosp * sinl - sine * sinp);
        moon.z = rm * (sine * cosp * sinl + cose * sinp);
    }

    return true;
}

static bool eci_2_ecef(ts::Tai const& time, Float3 const& eci, Float3& ecef) {
    VSCOPE_FUNCTIONF("%s, (%f, %f, %f)", time.rtklib_time_string().c_str(), eci.x, eci.y, eci.z);

    // TODO(ewasjon): Earth rotation angle
    auto xp      = 0.0;
    auto yp      = 0.0;
    auto ut1_utc = 0.0;

    // Get Terrestrial Time (TT) from TAI
    auto t_s = time.timestamp().full_seconds() + 32.184 /* TT - TAI */;
    auto t_c = t_s / 86400.0 / 36525.0;

    auto t  = t_c;
    auto t2 = t * t;
    auto t3 = t2 * t;

    // IAU 1976 Precession
    auto ze  = (2306.2181 * t + 0.30188 * t2 + 0.017998 * t3) * constant::ARCSEC2RAD;
    auto th  = (2004.3109 * t - 0.42665 * t2 - 0.041833 * t3) * constant::ARCSEC2RAD;
    auto z   = (2306.2181 * t + 1.09468 * t2 + 0.018203 * t3) * constant::ARCSEC2RAD;
    auto eps = (84381.448 - 46.8150 * t - 0.00059 * t2 + 0.001813 * t3) * constant::ARCSEC2RAD;

    auto p = Mat3::rotate_z(-ze) * Mat3::rotate_y(th) * Mat3::rotate_z(-ze);

    // IAU 1980 Nutation
    AstronomicalArguments args{};
    if (!compute_astronomical_arguments(t, args)) {
        VERBOSEF("failed to compute astronomical arguments");
        return false;
    }

    Iau1980Nutation nutation{};
    if (!compute_iau1980_nutation(t, args, nutation)) {
        VERBOSEF("failed to compute IAU 1980 nutation");
        return false;
    }

    auto n = Mat3::rotate_x(-eps - nutation.d_eps) * Mat3::rotate_z(-nutation.d_psi) *
             Mat3::rotate_x(eps);

    // Greenwich Mean Sidereal Time
    auto gmst = ts::Utc{time}.gmst(ut1_utc);
    auto gast = gmst + nutation.d_psi * cos(eps);
    gast += (0.00264 * sin(args.d) + 0.000063 * sin(2.0 * args.d)) * constant::ARCSEC2RAD;

    // ECI to ECEF matrix
    auto w = Mat3::rotate_y(-xp) * Mat3::rotate_x(-yp);
    auto u = w * Mat3::rotate_z(gast) * n * p;

    ecef = u * eci;
    return true;
}

static bool compute_sun_and_moon_position_ecef(ts::Tai const& time, Float3& sun,
                                               Float3& moon) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    if (!compute_sun_and_moon_position_eci(time, sun, moon)) {
        return false;
    }

    return true;
}

static bool compute_solid_tide_pole(ts::Tai const& time, Float3 const& ground, Float3 const& body,
                                    double g_constant, Float3& pole) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    VERBOSEF("ground: (%f, %f, %f)", ground.x, ground.y, ground.z);
    VERBOSEF("body:   (%f, %f, %f)", body.x, body.y, body.z);

    auto gr = ground.length_squared();

    auto ground_unit = ground;
    if (!ground_unit.normalize()) {
        VERBOSEF("failed to ...");
        return false;
    }

    auto body_unit = body;
    if (!body_unit.normalize()) {
        VERBOSEF("failed to ...");
        return false;
    }

    auto gm  = constant::GME / g_constant;
    auto re4 = constant::RE_WGS84 * constant::RE_WGS84 * constant::RE_WGS84 * constant::RE_WGS84;
    auto gr3 = gr * gr * gr;
    auto k2  = gm * re4 / gr3;

    auto h2 = 0.6078;
    auto l2 = 0.0847;

    auto a  = dot_product(body_unit, ground_unit);
    auto dp = k2 * 3.0 * l2 * a;
    auto du = k2 * (h2 * (1.5 * a * a - 0.5) - 3.0 * l2 * a * a);

    pole.x = dp * body_unit.x + du * ground_unit.x;
    pole.y = dp * body_unit.y + du * ground_unit.y;
    pole.z = dp * body_unit.z + du * ground_unit.z;
    return true;
}

void Observation::compute_earth_solid_tides() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    Float3 sun{};
    Float3 moon{};
    if (!compute_sun_and_moon_position_ecef(mReceptionTime, sun, moon)) {
        VERBOSEF("failed to compute sun and moon position");
        return;
    }

    Float3 sun_pole{};
    if (!compute_solid_tide_pole(mReceptionTime, mGroundPosition, sun,
                                 constant::SUN_GRAVITATIONAL_CONSTANT, sun_pole)) {
        VERBOSEF("failed to compute solid tide pole");
        return;
    }

    Float3 moon_pole{};
    if (!compute_solid_tide_pole(mReceptionTime, mGroundPosition, moon,
                                 constant::MOON_GRAVITATIONAL_CONSTANT, moon_pole)) {
        VERBOSEF("failed to compute solid tide pole");
        return;
    }

    Float3 delta{};
    delta.x = sun_pole.x + moon_pole.x;
    delta.y = sun_pole.y + moon_pole.y;
    delta.z = sun_pole.z + moon_pole.z;

    mSolidTidesDisplacement = delta;
    mSolidTides             = dot_product(mLineOfSight, mSolidTidesDisplacement);
    mSolidTidesValid        = true;

    VERBOSEF("disp x: %+.14f", mSolidTidesDisplacement.x);
    VERBOSEF("disp y: %+.14f", mSolidTidesDisplacement.y);
    VERBOSEF("disp z: %+.14f", mSolidTidesDisplacement.z);
}

void Observation::compute_antenna_phase_variation() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
}

void Observation::compute_code_range() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    VERBOSEF("true_range:   %+24.10f", mTrueRange);

    auto clock_bias = constant::SPEED_OF_LIGHT * mEphClockBias;
    VERBOSEF("clock_bias:   %+24.10f (%gs)", clock_bias, mEphClockBias);

    auto clock = 0.0;
    if (mClockCorrectionValid) {
        clock = mClockCorrection;
        VERBOSEF("clock:        %+24.10f (%gs)", clock,
                 mClockCorrection / constant::SPEED_OF_LIGHT);
    } else {
        VERBOSEF("clock:        ---");
    }

    auto code_bias = 0.0;
    if (mCodeBiasValid) {
        code_bias = mCodeBias;
        VERBOSEF("code_bias:    %+24.10f (%gm)", code_bias, mCodeBias);
    } else {
        VERBOSEF("code_bias:    ---");
    }

    auto stec_grid = 0.0;
    auto stec_poly = 0.0;
    if (mIonoValid) {
        stec_grid = 40.3e10 * mIonoGridResidual / (mFrequency * mFrequency);
        stec_poly = 40.3e10 * mIonoPolyResidual / (mFrequency * mFrequency);
        VERBOSEF("stec_grid:    %+24.10f (%g TECU, %g kHz)", stec_grid, mIonoGridResidual,
                 mFrequency);
        VERBOSEF("stec_poly:    %+24.10f (%g TECU, %g kHz)", stec_poly, mIonoPolyResidual,
                 mFrequency);
    } else {
        VERBOSEF("stec_grid:    ---");
        VERBOSEF("stec_poly:    ---");
    }

    auto tropo_wet_height_correction = 1.0;
    auto tropo_dry_height_correction = 1.0;
    if (mTropeHeightCorrectionValid) {
        tropo_dry_height_correction = mTropoDryHeightCorrection;
        tropo_wet_height_correction = mTropoWetHeightCorrection;
    }

    auto tropo_wet = 0.0;
    auto tropo_dry = 0.0;
    if (mTropoValid) {
        tropo_dry = mTropoDry * mTropoDryMapping * tropo_dry_height_correction;
        tropo_wet = mTropoWet * mTropoWetMapping * tropo_wet_height_correction;
        VERBOSEF("tropo_dry:    %+24.10f (%gm x %g x %g)", tropo_dry, mTropoDry, mTropoDryMapping,
                 tropo_dry_height_correction);
        VERBOSEF("tropo_wet:    %+24.10f (%gm x %g x %g)", tropo_wet, mTropoWet, mTropoWetMapping,
                 tropo_wet_height_correction);
    } else {
        VERBOSEF("tropo_dry:    ---");
        VERBOSEF("tropo_wet:    ---");
    }

    auto shapiro = 0.0;
    if (mShapiroValid) {
        shapiro = mShapiro;
        VERBOSEF("shapiro:      %+24.10f (%gm)", shapiro, mShapiro);
    } else {
        VERBOSEF("shapiro:      ---");
    }

    auto solid_tides = 0.0;
    if (mSolidTidesValid) {
        solid_tides = mSolidTides;
        VERBOSEF("solid_tides:  %+24.10f (%gm)", solid_tides, mSolidTides);
    } else {
        VERBOSEF("solid_tides:  ---");
    }

    auto pseudo_correction = clock + code_bias + stec_grid + stec_poly + tropo_dry + tropo_wet;
    auto final_correction  = shapiro + solid_tides;
    auto result            = mTrueRange + clock_bias + pseudo_correction + final_correction;
    DEBUGF("%s(%s): code  %+24.10f (%g, %g)", mSvId.name(), mSignalId.name(), result,
           pseudo_correction, final_correction);

    mCodeRange = result;
}

void Observation::compute_phase_range() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    VERBOSEF("true_range:   %+24.10f", mTrueRange);

    auto clock_bias = constant::SPEED_OF_LIGHT * mEphClockBias;
    VERBOSEF("clock_bias:   %+24.10f (%gs)", clock_bias, mEphClockBias);

    auto clock = 0.0;
    if (mClockCorrectionValid) {
        clock = mClockCorrection;
        VERBOSEF("clock:        %+24.10f (%gs)", clock,
                 mClockCorrection / constant::SPEED_OF_LIGHT);
    } else {
        VERBOSEF("clock:        ---");
    }

    auto phase_bias = 0.0;
    if (mPhaseBiasValid) {
        phase_bias = mPhaseBias;
        VERBOSEF("phase_bias:   %+24.10f (%gm)", phase_bias, mPhaseBias);
    } else {
        VERBOSEF("phase_bias:   ---");
    }

    auto stec_grid = 0.0;
    auto stec_poly = 0.0;
    if (mIonoValid) {
        stec_grid = -40.3e10 * mIonoGridResidual / (mFrequency * mFrequency);
        stec_poly = -40.3e10 * mIonoPolyResidual / (mFrequency * mFrequency);
        VERBOSEF("stec_grid:    %+24.10f (%g TECU, %g kHz)", stec_grid, mIonoGridResidual,
                 mFrequency);
        VERBOSEF("stec_poly:    %+24.10f (%g TECU, %g kHz)", stec_poly, mIonoPolyResidual,
                 mFrequency);
    } else {
        VERBOSEF("stec_grid:    ---");
        VERBOSEF("stec_poly:    ---");
    }

    auto tropo_wet_height_correction = 1.0;
    auto tropo_dry_height_correction = 1.0;
    if (mTropeHeightCorrectionValid) {
        tropo_dry_height_correction = mTropoDryHeightCorrection;
        tropo_wet_height_correction = mTropoWetHeightCorrection;
    }

    auto tropo_wet = 0.0;
    auto tropo_dry = 0.0;
    if (mTropoValid) {
        tropo_dry = mTropoDry * mTropoDryMapping * tropo_dry_height_correction;
        tropo_wet = mTropoWet * mTropoWetMapping * tropo_wet_height_correction;
        VERBOSEF("tropo_dry:    %+24.10f (%gm x %g x %g)", tropo_dry, mTropoDry, mTropoDryMapping,
                 tropo_dry_height_correction);
        VERBOSEF("tropo_wet:    %+24.10f (%gm x %g x %g)", tropo_wet, mTropoWet, mTropoWetMapping,
                 tropo_wet_height_correction);
    } else {
        VERBOSEF("tropo_dry:    ---");
        VERBOSEF("tropo_wet:    ---");
    }

    auto shapiro = 0.0;
    if (mShapiroValid) {
        shapiro = mShapiro;
        VERBOSEF("shapiro:      %+24.10f (%gm)", shapiro, mShapiro);
    } else {
        VERBOSEF("shapiro:      ---");
    }

    auto solid_tides = 0.0;
    if (mSolidTidesValid) {
        solid_tides = mSolidTides;
        VERBOSEF("solid_tides:  %+24.10f (%gm)", solid_tides, mSolidTides);
    } else {
        VERBOSEF("solid_tides:  ---");
    }

    auto phase_windup = 0.0;
    if (mPhaseWindupValid) {
        phase_windup = mPhaseWindup;
        VERBOSEF("phase_windup: %+24.10f (%gm)", phase_windup, mPhaseWindup);
    } else {
        VERBOSEF("phase_windup: ---");
    }

    auto antenna_phase_variation = 0.0;
    if (mAntennaPhaseVariationValid) {
        antenna_phase_variation = mAntennaPhaseVariation;
        VERBOSEF("ant_phase:    %+24.10f (%gm)", antenna_phase_variation, mAntennaPhaseVariation);
    } else {
        VERBOSEF("ant_phase:    ---");
    }

    auto carrier_correction = clock + phase_bias + stec_grid + stec_poly + tropo_dry + tropo_wet;
    auto final_correction   = shapiro + solid_tides + phase_windup + antenna_phase_variation;
    auto result             = mTrueRange + clock_bias + carrier_correction + final_correction;
    DEBUGF("%s(%s): phase %+24.10f (%g, %g)", mSvId.name(), mSignalId.name(), result,
           carrier_correction, final_correction);
    mPhaseRange = result;
}

double Observation::pseudorange() const NOEXCEPT {
    return mCodeRange;
}

double Observation::carrier_cycle() const NOEXCEPT {
    return mPhaseRange;
}

}  // namespace tokoro
}  // namespace generator
