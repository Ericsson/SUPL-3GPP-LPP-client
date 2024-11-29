#include "observation.hpp"
#include "coordinate.hpp"
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
      mCurrent{satellite.current_location()},
      mNext{satellite.next_location()} {
    mIsValid = true;

    // TODO(ewasjon): For GLONASS, the frequency depends on the channel number
    mFrequency  = signal_id.frequency();
    mWavelength = constant::SPEED_OF_LIGHT / mFrequency / 1000.0;

    mClockCorrection       = Correction{satellite.clock_correction(), true};
    mCodeBias              = Correction{0.0, false};
    mPhaseBias             = Correction{0.0, false};
    mTropospheric          = TroposphericDelay{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false, false};
    mIonospheric           = IonosphericDelay{0.0, 0.0, false};
    mShapiro               = Correction{0.0, false};
    mPhaseWindup           = Correction{0.0, false};
    mEarthSolidTides       = SolidEarthTides{0.0, {}, false};
    mAntennaPhaseVariation = Correction{0.0, false};

    mGroundPosition = location;
    mGroundLlh      = ecef_to_llh(location, WGS84);

    auto mapping =
        hydrostatic_mapping_function(mCurrent.reception_time, mGroundLlh, mCurrent.true_elevation);
    mTropospheric.mapping_hydrostatic = mapping.hydrostatic;
    mTropospheric.mapping_wet         = mapping.wet;
}

void Observation::compute_tropospheric_height() NOEXCEPT {
    VSCOPE_FUNCTION();

    auto ellipsoidal_height = mGroundLlh.z;
    auto geoid_height       = Geoid::height(mGroundLlh.x, mGroundLlh.y, Geoid::Model::EMBEDDED);

    HydrostaticAndWetDelay alt_0{};
    HydrostaticAndWetDelay alt_eh{};
    auto                   mops_0 =
        mops_tropospheric_delay(mCurrent.reception_time, mGroundLlh.x, 0.0, geoid_height, alt_0);
    auto mops_eh = mops_tropospheric_delay(mCurrent.reception_time, mGroundLlh.x,
                                           ellipsoidal_height, geoid_height, alt_eh);
    if (mops_0 && mops_eh) {
        mTropospheric.height_mapping_hydrostatic = alt_eh.hydrostatic / alt_0.hydrostatic;
        mTropospheric.height_mapping_wet         = alt_eh.wet / alt_0.wet;
        mTropospheric.valid_height_mapping       = true;
    } else {
        WARNF("failed to compute tropospheric height correction");
        mTropospheric.valid_height_mapping = false;
    }
}

void Observation::compute_phase_bias(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    auto signals = correction_data.signal_corrections(mSvId);
    if (!signals) return;

    auto it = signals->phase_bias.find(mSignalId);
    if (it == signals->phase_bias.end()) return;
    mPhaseBias = Correction{it->second.bias, true};
}

void Observation::compute_code_bias(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    auto signals = correction_data.signal_corrections(mSvId);
    if (!signals) return;

    auto it = signals->code_bias.find(mSignalId);
    if (it == signals->code_bias.end()) return;
    mCodeBias = Correction{it->second.bias, true};
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

    if (mTropospheric.valid) {
        VERBOSEF("tropospheric correction already computed");
        return;
    }

    mTropospheric.hydrostatic = correction.dry;
    mTropospheric.wet         = correction.wet;
    mTropospheric.valid       = true;
}

void Observation::compute_ionospheric(EcefPosition          location,
                                      CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    IonosphericCorrection correction{};
    if (!correction_data.ionospheric(mSvId, location, correction)) {
        VERBOSEF("ionospheric correction not found");
        return;
    }

    mIonospheric.grid_residual = correction.grid_residual;
    mIonospheric.poly_residual = correction.polynomial_residual;
    mIonospheric.valid         = true;
}

void Observation::compute_shapiro() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    auto r_sat = geocentric_distance(mCurrent.true_position);
    auto r_rcv = geocentric_distance(mGroundPosition);
    auto r     = mCurrent.true_range;

    // https://gssc.esa.int/navipedia/index.php/Relativistic_Path_Range_Effect
    auto shapiro = (2 * constant::GME / (constant::SPEED_OF_LIGHT * constant::SPEED_OF_LIGHT)) *
                   log((r_sat + r_rcv + r) / (r_sat + r_rcv - r));

    mShapiro = Correction{shapiro, true};
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
    VERBOSEF("t_js: %f", t_js);

    // Get the Julian centuries since J2000
    auto t_jc = t_js / 86400.0 / 36525.0;
    VERBOSEF("t_jc: %f", t_jc);

    // Get astronomical arguments
    AstronomicalArguments args{};
    if (!compute_astronomical_arguments(t_jc, args)) {
        VERBOSEF("failed to compute astronomical arguments");
        return false;
    }

    VERBOSEF("astronomical arguments:");
    VERBOSEF("  l: %f", args.l);
    VERBOSEF("  lp: %f", args.lp);
    VERBOSEF("  f: %f", args.f);
    VERBOSEF("  d: %f", args.d);
    VERBOSEF("  omega: %f", args.omega);

    // Obliquity of the ecliptic
    auto epsilon = 23.439291 - 0.0130042 * t_jc;
    auto sine    = sin(epsilon * constant::DEG2RAD);
    auto cose    = cos(epsilon * constant::DEG2RAD);
    VERBOSEF("epsilon: %f", epsilon);
    VERBOSEF("sine: %f", sine);
    VERBOSEF("cose: %f", cose);

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
        sun.y = rs * cose * sinl;
        sun.z = rs * sine * sinl;
        VERBOSEF("sun: (%f, %f, %f)", sun.x, sun.y, sun.z);
    }

    {
        // Moon
        auto lm = 218.32 + 481267.883 * t_jc + 6.29 * sin(args.l) -
                  1.27 * sin(args.l - 2.0 * args.d) + 0.66 * sin(2.0 * args.d) +
                  0.21 * sin(2.0 * args.l) - 0.19 * sin(args.lp) - 0.11 * sin(2.0 * args.f);
        auto pm = 5.13 * sin(args.f) + 0.28 * sin(args.l + args.f) - 0.28 * sin(args.f - args.l) -
                  0.17 * sin(args.f - 2.0 * args.d);
        auto rm = constant::RE_WGS84 /
                  sin((0.9508 + 0.0518 * cos(args.l) + 0.0095 * cos(args.l - 2.0 * args.d) +
                       0.0078 * cos(2.0 * args.d) + 0.0028 * cos(2.0 * args.l)) *
                      constant::DEG2RAD);

        auto sinl = sin(lm * constant::DEG2RAD);
        auto cosl = cos(lm * constant::DEG2RAD);
        auto sinp = sin(pm * constant::DEG2RAD);
        auto cosp = cos(pm * constant::DEG2RAD);

        moon.x = rm * cosp * cosl;
        moon.y = rm * (cose * cosp * sinl - sine * sinp);
        moon.z = rm * (sine * cosp * sinl + cose * sinp);
        VERBOSEF("moon: (%f, %f, %f)", moon.x, moon.y, moon.z);
    }

    return true;
}

static bool eci_2_ecef(ts::Tai const& time, Mat3& transform, double* gmst_out) {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    // TODO(ewasjon): Earth rotation angle
    auto xp      = 0.0;
    auto yp      = 0.0;
    auto ut1_utc = 0.0;

    // Get Terrestrial Time (TT) from TAI
    auto j2000     = ts::Utc::from_date_time(2000, 1, 1, 12, 0, 0);
    auto j2000_tai = ts::Tai{j2000};
    auto t_s       = (time.timestamp().full_seconds() - j2000_tai.timestamp().full_seconds()) +
               32.184 /* TT - TAI */;
    auto t_c = t_s / 86400.0 / 36525.0;

    auto t  = t_c;
    auto t2 = t * t;
    auto t3 = t2 * t;
    VERBOSEF("t: %f", t);
    VERBOSEF("t2: %f", t2);
    VERBOSEF("t3: %f", t3);

    // IAU 1976 Precession
    auto ze  = (2306.2181 * t + 0.30188 * t2 + 0.017998 * t3) * constant::ARCSEC2RAD;
    auto th  = (2004.3109 * t - 0.42665 * t2 - 0.041833 * t3) * constant::ARCSEC2RAD;
    auto z   = (2306.2181 * t + 1.09468 * t2 + 0.018203 * t3) * constant::ARCSEC2RAD;
    auto eps = (84381.448 - 46.8150 * t - 0.00059 * t2 + 0.001813 * t3) * constant::ARCSEC2RAD;
    VERBOSEF("ze: %f", ze);
    VERBOSEF("th: %f", th);
    VERBOSEF("z: %f", z);
    VERBOSEF("eps: %f", eps);

    auto p = Mat3::rotate_z(-z) * Mat3::rotate_y(th) * Mat3::rotate_z(-ze);

    VERBOSEF("p:");
    VERBOSEF("  %+.14f %+.14f %+.14f", p.m[0], p.m[1], p.m[2]);
    VERBOSEF("  %+.14f %+.14f %+.14f", p.m[3], p.m[4], p.m[5]);
    VERBOSEF("  %+.14f %+.14f %+.14f", p.m[6], p.m[7], p.m[8]);

    // IAU 1980 Nutation
    AstronomicalArguments args{};
    if (!compute_astronomical_arguments(t, args)) {
        VERBOSEF("failed to compute astronomical arguments");
        return false;
    }

    VERBOSEF("astronomical arguments:");
    VERBOSEF("  l: %f", args.l);
    VERBOSEF("  lp: %f", args.lp);
    VERBOSEF("  f: %f", args.f);
    VERBOSEF("  d: %f", args.d);
    VERBOSEF("  omega: %f", args.omega);

    Iau1980Nutation nutation{};
    if (!compute_iau1980_nutation(t, args, nutation)) {
        VERBOSEF("failed to compute IAU 1980 nutation");
        return false;
    }

    VERBOSEF("nutation:");
    VERBOSEF("  d_psi: %+.14f", nutation.d_psi);
    VERBOSEF("  d_eps: %+.14f", nutation.d_eps);

    VERBOSEF("  sin(-d_psi): %+.14f", sin(-nutation.d_psi));

    auto n1 = Mat3::rotate_x(-eps - nutation.d_eps);
    auto n2 = Mat3::rotate_z(-nutation.d_psi);
    auto n3 = Mat3::rotate_x(eps);

    auto r = n1 * n2;
    auto n = r * n3;

    VERBOSEF("n1:");
    VERBOSEF("  %+.14f %+.14f %+.14f", n1.m[0], n1.m[1], n1.m[2]);
    VERBOSEF("  %+.14f %+.14f %+.14f", n1.m[3], n1.m[4], n1.m[5]);
    VERBOSEF("  %+.14f %+.14f %+.14f", n1.m[6], n1.m[7], n1.m[8]);
    VERBOSEF("n2:");
    VERBOSEF("  %+.14f %+.14f %+.14f", n2.m[0], n2.m[1], n2.m[2]);
    VERBOSEF("  %+.14f %+.14f %+.14f", n2.m[3], n2.m[4], n2.m[5]);
    VERBOSEF("  %+.14f %+.14f %+.14f", n2.m[6], n2.m[7], n2.m[8]);
    VERBOSEF("n3:");
    VERBOSEF("  %+.14f %+.14f %+.14f", n3.m[0], n3.m[1], n3.m[2]);
    VERBOSEF("  %+.14f %+.14f %+.14f", n3.m[3], n3.m[4], n3.m[5]);
    VERBOSEF("  %+.14f %+.14f %+.14f", n3.m[6], n3.m[7], n3.m[8]);
    VERBOSEF("r:");
    VERBOSEF("  %+.14f %+.14f %+.14f", r.m[0], r.m[1], r.m[2]);
    VERBOSEF("  %+.14f %+.14f %+.14f", r.m[3], r.m[4], r.m[5]);
    VERBOSEF("  %+.14f %+.14f %+.14f", r.m[6], r.m[7], r.m[8]);

    VERBOSEF("n:");
    VERBOSEF("  %+.14f %+.14f %+.14f", n.m[0], n.m[1], n.m[2]);
    VERBOSEF("  %+.14f %+.14f %+.14f", n.m[3], n.m[4], n.m[5]);
    VERBOSEF("  %+.14f %+.14f %+.14f", n.m[6], n.m[7], n.m[8]);

    // Greenwich Mean Sidereal Time
    auto gmst = ts::Utc{time}.gmst(ut1_utc);
    auto gast = gmst + nutation.d_psi * cos(eps);
    gast += (0.00264 * sin(args.d) + 0.000063 * sin(2.0 * args.d)) * constant::ARCSEC2RAD;

    VERBOSEF("gmst: %f", gmst);
    VERBOSEF("gast: %f", gast);

    if (gmst_out) {
        *gmst_out = gmst;
    }

    // ECI to ECEF matrix
    auto w = Mat3::rotate_y(-xp) * Mat3::rotate_x(-yp);
    VERBOSEF("w:");
    VERBOSEF("  %+.14f %+.14f %+.14f", w.m[0], w.m[1], w.m[2]);
    VERBOSEF("  %+.14f %+.14f %+.14f", w.m[3], w.m[4], w.m[5]);
    VERBOSEF("  %+.14f %+.14f %+.14f", w.m[6], w.m[7], w.m[8]);

    auto u = (w * Mat3::rotate_z(gast)) * (n * p);

    VERBOSEF("u:");
    VERBOSEF("  %+.4f %+.4f %+.4f", u.m[0], u.m[1], u.m[2]);
    VERBOSEF("  %+.4f %+.4f %+.4f", u.m[3], u.m[4], u.m[5]);
    VERBOSEF("  %+.4f %+.4f %+.4f", u.m[6], u.m[7], u.m[8]);

    transform = u;
    return true;
}

static bool compute_sun_and_moon_position_ecef(ts::Tai const& time, Float3& sun_ecef,
                                               Float3& moon_ecef, double* gmst) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    Float3 sun_eci{};
    Float3 moon_eci{};
    if (!compute_sun_and_moon_position_eci(time, sun_eci, moon_eci)) {
        return false;
    }

    Mat3 transform{};
    if (!eci_2_ecef(time, transform, gmst)) {
        return false;
    }

    sun_ecef  = transform * sun_eci;
    moon_ecef = transform * moon_eci;
    return true;
}

static bool compute_solid_tide_pole(ts::Tai const& time, Float3 const& up, Float3 const& body,
                                    double g_constant, Float3& pole) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    VERBOSEF("up:     (%+.4f, %+.4f, %+.4f)", up.x, up.y, up.z);
    VERBOSEF("body:   (%+.4f, %+.4f, %+.4f)", body.x, body.y, body.z);

    auto body_distance = body.length();
    auto body_unit     = body;
    if (!body_unit.normalize()) {
        VERBOSEF("failed to ...");
        return false;
    }

    VERBOSEF("body_unit:     (%+.4f, %+.4f, %+.4f)", body_unit.x, body_unit.y, body_unit.z);
    VERBOSEF("body_distance: %+.4f", body_distance);

    auto gm  = g_constant / constant::GME;
    auto re4 = constant::RE_WGS84 * constant::RE_WGS84 * constant::RE_WGS84 * constant::RE_WGS84;
    auto br3 = body_distance * body_distance * body_distance;
    auto k2  = gm * re4 / br3;
    VERBOSEF("k2: %+.14f", k2);

    auto h2 = 0.6078;
    auto l2 = 0.0847;
    VERBOSEF("h2: %+.14f", h2);
    VERBOSEF("l2: %+.14f", l2);

    auto a  = dot_product(body_unit, up);
    auto dp = k2 * 3.0 * l2 * a;
    auto du = k2 * (h2 * (1.5 * a * a - 0.5) - 3.0 * l2 * a * a);
    VERBOSEF("dp: %+.14f", dp);
    VERBOSEF("du: %+.14f", du);

    pole.x = dp * body_unit.x + du * up.x;
    pole.y = dp * body_unit.y + du * up.y;
    pole.z = dp * body_unit.z + du * up.z;

    VERBOSEF("pole: (%+.14f, %+.14f, %+.14f)", pole.x, pole.y, pole.z);
    return true;
}

static bool compute_enu_basis(Float3 location, Float3& east, Float3& north, Float3& up) NOEXCEPT {
    VSCOPE_FUNCTIONF("%+.4f, %+.4f, %+.4f", location.x, location.y, location.z);

    auto r = location.length();

    // Calculate geodetic latitude and longitude assuming a spherical Earth
    auto lat = asin(location.z / r);
    auto lon = atan2(location.y, location.x);

    auto cos_lat = cos(lat);
    auto sin_lat = sin(lat);
    auto cos_lon = cos(lon);
    auto sin_lon = sin(lon);

    east.x = -sin_lon;
    east.y = cos_lon;
    east.z = 0.0;

    north.x = -sin_lat * cos_lon;
    north.y = -sin_lat * sin_lon;
    north.z = cos_lat;

    up.x = cos_lat * cos_lon;
    up.y = cos_lat * sin_lon;
    up.z = sin_lat;

    VERBOSEF("east:  (%+.4f, %+.4f, %+.4f)", east.x, east.y, east.z);
    VERBOSEF("north: (%+.4f, %+.4f, %+.4f)", north.x, north.y, north.z);
    VERBOSEF("up:    (%+.4f, %+.4f, %+.4f)", up.x, up.y, up.z);
    return true;
}

void Observation::compute_earth_solid_tides() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    Float3 east{};
    Float3 north{};
    Float3 up{};
    if (!compute_enu_basis(mGroundPosition, east, north, up)) {
        VERBOSEF("failed to compute ENU basis");
        return;
    }

    Float3 sun{};
    Float3 moon{};
    double gmst = 0.0;
    if (!compute_sun_and_moon_position_ecef(mCurrent.reception_time, sun, moon, &gmst)) {
        VERBOSEF("failed to compute sun and moon position");
        return;
    }

    Float3 sun_pole{};
    if (!compute_solid_tide_pole(mCurrent.reception_time, up, sun,
                                 constant::SUN_GRAVITATIONAL_CONSTANT, sun_pole)) {
        VERBOSEF("failed to compute solid tide pole");
        return;
    }

    Float3 moon_pole{};
    if (!compute_solid_tide_pole(mCurrent.reception_time, up, moon,
                                 constant::MOON_GRAVITATIONAL_CONSTANT, moon_pole)) {
        VERBOSEF("failed to compute solid tide pole");
        return;
    }

    auto sin2l    = std::sin(2.0 * mGroundLlh.x * constant::DEG2RAD);
    auto delta_up = -0.012 * sin2l * std::sin(gmst + mGroundLlh.y * constant::DEG2RAD);
    VERBOSEF("ground: %+.4f, %+.4f, %+.4f", mGroundLlh.x * constant::DEG2RAD,
             mGroundLlh.y * constant::DEG2RAD, mGroundLlh.z);
    VERBOSEF("sin2l: %+.14f", sin2l);
    VERBOSEF("delta_up: %+.14f", delta_up);

    mEarthSolidTides.displacement_vector = sun_pole + moon_pole + delta_up * up;
    mEarthSolidTides.displacement =
        -dot_product(mCurrent.true_line_of_sight, mEarthSolidTides.displacement_vector);
    mEarthSolidTides.valid = true;

    VERBOSEF("disp x: %+.14f * %+.14f = %+.14f", mEarthSolidTides.displacement_vector.x,
             mCurrent.true_line_of_sight.x,
             mEarthSolidTides.displacement_vector.x * mCurrent.true_line_of_sight.x);
    VERBOSEF("disp y: %+.14f * %+.14f = %+.14f", mEarthSolidTides.displacement_vector.y,
             mCurrent.true_line_of_sight.y,
             mEarthSolidTides.displacement_vector.y * mCurrent.true_line_of_sight.y);
    VERBOSEF("disp z: %+.14f * %+.14f = %+.14f", mEarthSolidTides.displacement_vector.z,
             mCurrent.true_line_of_sight.z,
             mEarthSolidTides.displacement_vector.z * mCurrent.true_line_of_sight.z);
    VERBOSEF("solid tides: %+.14f", mEarthSolidTides.displacement);
}

void Observation::compute_phase_windup() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    Float3 sun{};
    Float3 moon{};
    if (!compute_sun_and_moon_position_ecef(mCurrent.reception_time, sun, moon, nullptr)) {
        VERBOSEF("failed to compute sun and moon position");
        return;
    }

    auto k = mGroundPosition - mCurrent.true_position;
    if (!k.normalize()) return;
    VERBOSEF("k:   %+.4f, %+.4f, %+.4f", k.x, k.y, k.z);

    auto sz = -1.0 * mCurrent.true_position;
    if (!sz.normalize()) return;
    VERBOSEF("sz:  %+.4f, %+.4f, %+.4f", sz.x, sz.y, sz.z);

    auto we = Float3{0.0, 0.0, constant::EARTH_ANGULAR_VELOCITY};

    auto e = sun - mCurrent.true_position;
    if (!e.normalize()) return;

    auto o_cross_r = cross_product(we, mCurrent.true_position);
    auto ss        = cross_product(k, e);
    if (!ss.normalize()) return;
    VERBOSEF("ss:  %+.4f, %+.4f, %+.4f", ss.x, ss.y, ss.z);

    auto sy = cross_product(sz, ss);
    if (!sy.normalize()) return;
    VERBOSEF("sy:  %+.4f, %+.4f, %+.4f", sy.x, sy.y, sy.z);

    auto sx = cross_product(sy, sz);
    VERBOSEF("sx:  %+.4f, %+.4f, %+.4f", sx.x, sx.y, sx.z);

    auto sb = cross_product(k, e);
    auto sa = cross_product(sb, k);

    Float3 east, north, up;
    compute_enu_basis(mGroundLlh, east, north, up);

    auto rx = north;
    auto ry = -1.0 * east;
    VERBOSEF("rx:  %+.4f, %+.4f, %+.4f", rx.x, rx.y, rx.z);
    VERBOSEF("ry:  %+.4f, %+.4f, %+.4f", ry.x, ry.y, ry.z);

    auto ra = rx;
    auto rb = ry;

    auto rd = ra - k * dot_product(k, ra) + cross_product(k, rb);
    auto sd = sa - k * dot_product(k, sa) - cross_product(k, sb);
    VERBOSEF("sd:  %+.4f, %+.4f, %+.4f", sd.x, sd.y, sd.z);
    VERBOSEF("rd:  %+.4f, %+.4f, %+.4f", rd.x, rd.y, rd.z);
    auto cosp = dot_product(sd, rd) / (rd.length() * sd.length());
    if (cosp < -1.0) cosp = -1.0;
    if (cosp > 1.0) cosp = 1.0;
    VERBOSEF("cos: %+.4f", cosp);
    auto ph = acos(cosp) / (2.0 * constant::PI);
    VERBOSEF("ph:  %+.4f", ph);
    if (dot_product(k, cross_product(sd, rd)) < 0.0) ph = -ph;

    auto prev_phw = 0.0;
    VERBOSEF("phw: %+.4f", prev_phw);
    auto phw = ph + floor(prev_phw - ph + 0.5);
    VERBOSEF("phw: %+.4f (%+.4f)", phw, ph);

    mPhaseWindup.correction = phw;
    mPhaseWindup.valid      = true;
}

void Observation::compute_antenna_phase_variation() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    mAntennaPhaseVariation.valid = false;
}

void Observation::compute_ranges() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    VERBOSEF("true_range:   %+24.10f", mCurrent.true_range);

    auto clock_bias = constant::SPEED_OF_LIGHT * -mCurrent.eph_clock_bias;
    VERBOSEF("clock_bias:   %+24.10f (%gs)", clock_bias, -mCurrent.eph_clock_bias);

    auto clock = 0.0;
    if (mClockCorrection.valid) {
        clock = mClockCorrection.correction;
        VERBOSEF("clock:        %+24.10f (%gs)", clock,
                 mClockCorrection.correction / constant::SPEED_OF_LIGHT);
    } else {
        VERBOSEF("clock:        ---");
    }

    auto code_bias = 0.0;
    if (mCodeBias.valid) {
        code_bias = mCodeBias.correction;
        VERBOSEF("code_bias:    %+24.10f (%gm)", code_bias, mCodeBias.correction);
    } else {
        VERBOSEF("code_bias:    ---");
    }

    auto phase_bias = 0.0;
    if (mPhaseBias.valid) {
        phase_bias = mPhaseBias.correction;
        VERBOSEF("phase_bias:   %+24.10f (%gm)", phase_bias, mPhaseBias.correction);
    } else {
        VERBOSEF("phase_bias:   ---");
    }

    auto stec_grid = 0.0;
    auto stec_poly = 0.0;
    if (mIonospheric.valid) {
        stec_grid = 40.3e10 * mIonospheric.grid_residual / (mFrequency * mFrequency);
        stec_poly = 40.3e10 * mIonospheric.poly_residual / (mFrequency * mFrequency);
        VERBOSEF("stec_grid:    %+24.10f (%g TECU, %g kHz)", stec_grid, mIonospheric.grid_residual,
                 mFrequency);
        VERBOSEF("stec_poly:    %+24.10f (%g TECU, %g kHz)", stec_poly, mIonospheric.poly_residual,
                 mFrequency);
    } else {
        VERBOSEF("stec_grid:    ---");
        VERBOSEF("stec_poly:    ---");
    }

    auto tropo_wet_height_correction = 1.0;
    auto tropo_dry_height_correction = 1.0;
    if (mTropospheric.valid_height_mapping) {
        tropo_dry_height_correction = mTropospheric.height_mapping_hydrostatic;
        tropo_wet_height_correction = mTropospheric.height_mapping_wet;
    }

    auto tropo_wet = 0.0;
    auto tropo_dry = 0.0;
    if (mTropospheric.valid) {
        tropo_dry = mTropospheric.hydrostatic * mTropospheric.mapping_hydrostatic *
                    tropo_dry_height_correction;
        tropo_wet = mTropospheric.wet * mTropospheric.mapping_wet * tropo_wet_height_correction;
        VERBOSEF("tropo_dry:    %+24.10f (%gm x %g x %g)", tropo_dry, mTropospheric.hydrostatic,
                 mTropospheric.mapping_hydrostatic, tropo_dry_height_correction);
        VERBOSEF("tropo_wet:    %+24.10f (%gm x %g x %g)", tropo_wet, mTropospheric.wet,
                 mTropospheric.mapping_wet, tropo_wet_height_correction);
    } else {
        VERBOSEF("tropo_dry:    ---");
        VERBOSEF("tropo_wet:    ---");
    }

    auto shapiro = 0.0;
    if (mShapiro.valid) {
        shapiro = mShapiro.correction;
        VERBOSEF("shapiro:      %+24.10f (%gm)", shapiro, mShapiro.correction);
    } else {
        VERBOSEF("shapiro:      ---");
    }

    auto solid_tides = 0.0;
    if (mEarthSolidTides.valid) {
        solid_tides = mEarthSolidTides.displacement;
        VERBOSEF("solid_tides:  %+24.10f (%gm,(%g,%g,%g))", solid_tides,
                 mEarthSolidTides.displacement, mEarthSolidTides.displacement_vector.x,
                 mEarthSolidTides.displacement_vector.y, mEarthSolidTides.displacement_vector.z);
    } else {
        VERBOSEF("solid_tides:  ---");
    }

    auto phase_windup = 0.0;
    if (mPhaseWindup.valid) {
        phase_windup = mPhaseWindup.correction * mWavelength;
        VERBOSEF("phase_windup: %+24.10f (%gc x %g)", phase_windup, mPhaseWindup.correction,
                 mWavelength);
    } else {
        VERBOSEF("phase_windup: ---");
    }

    auto antenna_phase_variation = 0.0;
    if (mAntennaPhaseVariation.valid) {
        antenna_phase_variation = mAntennaPhaseVariation.correction;
        VERBOSEF("ant_phase:    %+24.10f (%gm)", antenna_phase_variation,
                 mAntennaPhaseVariation.correction);
    } else {
        VERBOSEF("ant_phase:    ---");
    }

    auto code_correction       = clock + code_bias + stec_grid + stec_poly + tropo_dry + tropo_wet;
    auto phase_correction      = clock + phase_bias - stec_grid - stec_poly + tropo_dry + tropo_wet;
    auto final_code_correction = shapiro + solid_tides;
    auto final_phase_correction = shapiro + solid_tides + phase_windup + antenna_phase_variation;
    auto code_result = mCurrent.true_range + clock_bias + code_correction + final_code_correction;
    auto phase_result =
        mCurrent.true_range + clock_bias + phase_correction + final_phase_correction;
    DEBUGF("%s(%s): code  %+24.10f (%g, %g)", mSvId.name(), mSignalId.name(), code_result,
           code_correction, final_code_correction);
    DEBUGF("%s(%s): phase %+24.10f (%g, %g)", mSvId.name(), mSignalId.name(), phase_result,
           phase_correction, final_phase_correction);

    auto next_phase_result =
        mNext.true_range + constant::SPEED_OF_LIGHT * -mNext.eph_clock_bias + phase_correction + final_phase_correction;
    auto phase_delta = next_phase_result - phase_result;
    auto time_delta = (mNext.reception_time.timestamp() - mCurrent.reception_time.timestamp()).full_seconds();
    auto phase_rate = phase_delta / time_delta;

    DEBUGF("%s(%s): phase rate %+19.10f (%g, %g)", mSvId.name(), mSignalId.name(), phase_rate,
           phase_delta, time_delta);


    INFOF(",%s,%s,%g,%u,%.14f,%.14f,%.14f,%.14f,%.14f,%.14f,%.14f,TOTAL,%.14f,%.14f,TOTAL,%.14f,%."
          "14f,%.14f,%.14f",
          mSvId.name(), mSignalId.name(), mFrequency, mIode, mCurrent.true_range, clock_bias, clock,
          code_bias, phase_bias, stec_grid, stec_poly, tropo_dry, tropo_wet, shapiro, solid_tides,
          phase_windup, antenna_phase_variation);


    mCodeRange      = code_result;
    mPhaseRange     = phase_result;
    mPhaseRangeRate = phase_rate;
}

double Observation::code_range() const NOEXCEPT {
    return mCodeRange;
}

double Observation::phase_range() const NOEXCEPT {
    return mPhaseRange;
}

}  // namespace tokoro
}  // namespace generator
