#include "observation.hpp"
#include "coordinate.hpp"
#include "coordinates/enu.hpp"
#include "coordinates/eci.hpp"
#include "data.hpp"
#include "models/astronomical_arguments.hpp"
#include "models/geoid.hpp"
#include "models/helper.hpp"
#include "models/mops.hpp"
#include "models/nutation.hpp"
#include "models/shapiro.hpp"
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
      mCurrent{&satellite.current_state()},
      mNext{&satellite.next_state()} {
    mIsValid = true;

    // TODO(ewasjon): For GLONASS, the frequency depends on the channel number
    mFrequency  = signal_id.frequency();
    mWavelength = constant::SPEED_OF_LIGHT / mFrequency / 1000.0;

    mClockCorrection       = Correction{satellite.clock_correction(), true};
    mCodeBias              = Correction{0.0, false};
    mPhaseBias             = Correction{0.0, false};
    mTropospheric          = TroposphericDelay{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false, false};
    mIonospheric           = IonosphericDelay{0.0, 0.0, false};
    mShapiro               = {};
    mPhaseWindup           = Correction{0.0, false};
    mEarthSolidTides       = SolidEarthTides{0.0, {}, false};
    mAntennaPhaseVariation = Correction{0.0, false};

    mGroundPosition = location;
    mGroundLlh      = ecef_to_llh(location, ellipsoid::WGS84);

    auto mapping = hydrostatic_mapping_function(mCurrent->reception_time, mGroundLlh,
                                                mCurrent->true_elevation);
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
        mops_tropospheric_delay(mCurrent->reception_time, mGroundLlh.x, 0.0, geoid_height, alt_0);
    auto mops_eh = mops_tropospheric_delay(mCurrent->reception_time, mGroundLlh.x,
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

void Observation::compute_tropospheric(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mSvId.name());

    TroposphericCorrection correction{};
    if (!correction_data.tropospheric(mSvId, mGroundLlh, correction)) {
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

void Observation::compute_ionospheric(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    IonosphericCorrection correction{};
    if (!correction_data.ionospheric(mSvId, mGroundLlh, correction)) {
        VERBOSEF("ionospheric correction not found");
        return;
    }

    mIonospheric.grid_residual = correction.grid_residual;
    mIonospheric.poly_residual = correction.polynomial_residual;
    mIonospheric.valid         = true;
}

void Observation::compute_shapiro() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    mShapiro = model_shapiro(*mCurrent, mGroundPosition);
}

#if 0
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
#endif

static bool compute_sun_and_moon_position_eci(ts::Tai const& time, Float3& sun,
                                              Float3& moon) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    // TODO(ewasjon): Need UT1-UTC correction term, usually given by Earth Orientation Parameters
    // (EOP). However, UTC is by definition maintained to be within +-0.9s of UT1, so it probably
    // doesn't matter for our purposes.
    auto ut1_utc = 0.0;
    auto t_jc    = ts::Utc{time}.j2000_century(ut1_utc);

    // Get astronomical arguments
    auto args = AstronomicalArguments::evaluate(t_jc);

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

static bool compute_sun_and_moon_position_ecef(ts::Tai const& time, Float3& sun_ecef,
                                               Float3& moon_ecef, double* gmst) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    Float3 sun_eci{};
    Float3 moon_eci{};
    if (!compute_sun_and_moon_position_eci(time, sun_eci, moon_eci)) {
        return false;
    }

    EciEarthParameters earth_params{};
    Mat3 transform{};
    eci_to_ecef_matrix(time, earth_params, &transform, gmst);

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

void Observation::compute_earth_solid_tides() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    Float3 east{};
    Float3 north{};
    Float3 up{};
    enu_basis_from_xyz(mGroundPosition, east, north, up);

    Float3 sun{};
    Float3 moon{};
    double gmst = 0.0;
    if (!compute_sun_and_moon_position_ecef(mCurrent->reception_time, sun, moon, &gmst)) {
        VERBOSEF("failed to compute sun and moon position");
        return;
    }

    Float3 sun_pole{};
    if (!compute_solid_tide_pole(mCurrent->reception_time, up, sun,
                                 constant::SUN_GRAVITATIONAL_CONSTANT, sun_pole)) {
        VERBOSEF("failed to compute solid tide pole");
        return;
    }

    Float3 moon_pole{};
    if (!compute_solid_tide_pole(mCurrent->reception_time, up, moon,
                                 constant::MOON_GRAVITATIONAL_CONSTANT, moon_pole)) {
        VERBOSEF("failed to compute solid tide pole");
        return;
    }

    auto sin2l    = std::sin(2.0 * mGroundLlh.x);
    auto delta_up = -0.012 * sin2l * std::sin(gmst + mGroundLlh.y);
    VERBOSEF("ground: %+.4f, %+.4f, %+.4f", mGroundLlh.x, mGroundLlh.y, mGroundLlh.z);
    VERBOSEF("sin2l: %+.14f", sin2l);
    VERBOSEF("delta_up: %+.14f", delta_up);

    mEarthSolidTides.displacement_vector = sun_pole + moon_pole + delta_up * up;
    mEarthSolidTides.displacement =
        -dot_product(mCurrent->true_line_of_sight, mEarthSolidTides.displacement_vector);
    mEarthSolidTides.valid = true;

    VERBOSEF("disp x: %+.14f * %+.14f = %+.14f", mEarthSolidTides.displacement_vector.x,
             mCurrent->true_line_of_sight.x,
             mEarthSolidTides.displacement_vector.x * mCurrent->true_line_of_sight.x);
    VERBOSEF("disp y: %+.14f * %+.14f = %+.14f", mEarthSolidTides.displacement_vector.y,
             mCurrent->true_line_of_sight.y,
             mEarthSolidTides.displacement_vector.y * mCurrent->true_line_of_sight.y);
    VERBOSEF("disp z: %+.14f * %+.14f = %+.14f", mEarthSolidTides.displacement_vector.z,
             mCurrent->true_line_of_sight.z,
             mEarthSolidTides.displacement_vector.z * mCurrent->true_line_of_sight.z);
    VERBOSEF("solid tides: %+.14f", mEarthSolidTides.displacement);
}

static bool compute_satellite_antenna_basis_sun(Float3 satellite_position, Float3 sun_position,
                                                Float3& x, Float3& y, Float3& z) NOEXCEPT {
    VSCOPE_FUNCTIONF("(%+.4f, %+.4f, %+.4f), (%+.4f, %+.4f, %+.4f)", satellite_position.x,
                     satellite_position.y, satellite_position.z, sun_position.x, sun_position.y,
                     sun_position.z);

    // unit vector of satellite antenna (pointing at the center of the Earth)
    auto sz = -1.0 * satellite_position;
    if (!sz.normalize()) return false;

    // unit vector from sun to satellite
    auto e = sun_position - satellite_position;
    if (!e.normalize()) return false;
    VERBOSEF("e:   %+.4f, %+.4f, %+.4f", e.x, e.y, e.z);

    // the satellite antenna basis
    auto sy = cross_product(sz, e);
    if (!sy.normalize()) return false;
    auto sx = cross_product(sy, sz);
    if (!sx.normalize()) return false;

    VERBOSEF("sx:  %+.4f, %+.4f, %+.4f", sx.x, sx.y, sx.z);
    VERBOSEF("sy:  %+.4f, %+.4f, %+.4f", sy.x, sy.y, sy.z);
    VERBOSEF("sz:  %+.4f, %+.4f, %+.4f", sz.x, sz.y, sz.z);

    x = sx;
    y = sy;
    z = sz;
    return true;
}

static bool compute_satellite_antenna_basis_velocity(Float3 ground_position,
                                                     Float3 satellite_position,
                                                     Float3 satellite_velocity, Float3& x,
                                                     Float3& y, Float3& z) NOEXCEPT {
    VSCOPE_FUNCTIONF("(%+.4f, %+.4f, %+.4f), (%+.4f, %+.4f, %+.4f)", satellite_position.x,
                     satellite_position.y, satellite_position.z, satellite_velocity.x,
                     satellite_velocity.y, satellite_velocity.z);

    // unit vector of satellite antenna (pointing at the center of the Earth)
    auto sz = satellite_position;
    if (!sz.normalize()) return false;
    sz = -1.0 * sz;

    // unit vector from ground to satellite
    auto k = ground_position - satellite_position;
    if (!k.normalize()) return false;
    VERBOSEF("k:   %+.4f, %+.4f, %+.4f", k.x, k.y, k.z);

    // unit vector of velocity to satellite position considering Earth rotation
    auto e = satellite_velocity + constant::EARTH_ANGULAR_VELOCITY *
                                      Float3{-satellite_position.x, satellite_position.y, 0.0};
    if (!e.normalize()) return false;
    VERBOSEF("e:   %+.4f, %+.4f, %+.4f", e.x, e.y, e.z);

    // the satellite antenna basis
    auto sy = cross_product(sz, e);
    if (!sy.normalize()) return false;
    auto sx = cross_product(sy, sz);
    if (!sx.normalize()) return false;

    VERBOSEF("sx:  %+.4f, %+.4f, %+.4f", sx.x, sx.y, sx.z);
    VERBOSEF("sy:  %+.4f, %+.4f, %+.4f", sy.x, sy.y, sy.z);
    VERBOSEF("sz:  %+.4f, %+.4f, %+.4f", sz.x, sz.y, sz.z);

    x = sx;
    y = sy;
    z = sz;
    return true;
}

// https://github.com/Azurehappen/Virtual-Network-DGNSS-Project/blob/5d0904aabab5880d807e92460ac540d456819329/VN_DGNSS_Server/rtklib/phase_windup.cpp#L308
static double satellite_yaw_angle(double beta, double mu) NOEXCEPT {
    VSCOPE_FUNCTIONF("%+.4f, %+.4f", beta, mu);
    if (fabs(beta) < 1.0e-12 && fabs(mu) < 1.0e-12) {
        return constant::PI;
    }

    return atan2(-tan(beta), sin(mu)) + constant::PI;
}

// https://github.com/Azurehappen/Virtual-Network-DGNSS-Project/blob/5d0904aabab5880d807e92460ac540d456819329/VN_DGNSS_Server/rtklib/phase_windup.cpp#L321
static bool compute_satellite_antenna_basis_yaw(ts::Tai const& emission_time,
                                                Float3 ground_position, Float3 satellite_position,
                                                Float3 satellite_velocity, Float3& x, Float3& y,
                                                Float3& z) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, (%+.4f, %+.4f, %+.4f), (%+.4f, %+.4f, %+.4f)",
                     emission_time.rtklib_time_string().c_str(), ground_position.x,
                     ground_position.y, ground_position.z, satellite_position.x,
                     satellite_position.y, satellite_position.z);

    Float3 sun_position{};
    Float3 moon_position{};
    if (!compute_sun_and_moon_position_ecef(emission_time, sun_position, moon_position, nullptr)) {
        VERBOSEF("failed to compute sun and moon position");
        return false;
    }

    auto satellite_unit = satellite_position;
    if (!satellite_unit.normalize()) return false;
    VERBOSEF("stu: %+.4f, %+.4f, %+.4f", satellite_unit.x, satellite_unit.y, satellite_unit.z);

    auto sun_unit = sun_position;
    if (!sun_unit.normalize()) return false;
    VERBOSEF("suu: %+.4f, %+.4f, %+.4f", sun_unit.x, sun_unit.y, sun_unit.z);

    auto velocity =
        satellite_velocity +
        constant::EARTH_ANGULAR_VELOCITY * Float3{-satellite_position.x, satellite_position.y, 0.0};

    // normal of the plane formed by the satellite position (from the earth center) and the velocity
    auto n = cross_product(satellite_position, velocity);
    auto p = cross_product(sun_position, n);
    if (!n.normalize()) return false;
    if (!p.normalize()) return false;

    VERBOSEF("n:   %+.4f, %+.4f, %+.4f", n.x, n.y, n.z);
    VERBOSEF("p:   %+.4f, %+.4f, %+.4f", p.x, p.y, p.z);

    auto b = constant::PI / 2.0 - acos(dot_product(sun_unit, n));
    VERBOSEF("b:   %+.4f", b);

    auto e = acos(dot_product(satellite_unit, p));
    VERBOSEF("e:   %+.4f", e);

    auto mu = constant::PI / 2.0;
    if (dot_product(satellite_unit, sun_unit) <= 0.0) {
        mu -= e;
    } else {
        mu += e;
    }

    if (mu < -constant::PI) mu += 2.0 * constant::PI;
    if (mu >= constant::PI) mu -= 2.0 * constant::PI;
    VERBOSEF("mu:  %+.4f", mu);

    auto yaw = satellite_yaw_angle(b, mu);
    VERBOSEF("yaw: %+.4f", yaw);

    auto k = cross_product(n, satellite_unit);
    if (!k.normalize()) return false;
    VERBOSEF("k:   %+.4f, %+.4f, %+.4f", k.x, k.y, k.z);

    auto cos_yaw = cos(yaw);
    auto sin_yaw = sin(yaw);
    auto rx      = Float3{
        -sin_yaw * k.x + cos_yaw * n.x,
        -sin_yaw * k.y + cos_yaw * n.y,
        -sin_yaw * k.z + cos_yaw * n.z,
    };
    auto ry = Float3{
        -cos_yaw * k.x - sin_yaw * n.x,
        -cos_yaw * k.y - sin_yaw * n.y,
        -cos_yaw * k.z - sin_yaw * n.z,
    };
    auto rz = cross_product(rx, ry);
    if (!rz.normalize()) return false;

    VERBOSEF("rx:  %+.4f, %+.4f, %+.4f", rx.x, rx.y, rx.z);
    VERBOSEF("ry:  %+.4f, %+.4f, %+.4f", ry.x, ry.y, ry.z);
    VERBOSEF("rz:  %+.4f, %+.4f, %+.4f", rz.x, rz.y, rz.z);

    x = rx;
    y = ry;
    z = rz;
    return true;
}

static bool compute_receiver_antenna_basis(Float3 ground_position_llh, Float3& x, Float3& y,
                                           Float3& z) NOEXCEPT {
    VSCOPE_FUNCTIONF("(%+.4f, %+.4f, %+.4f)", ground_position_llh.x * constant::RAD2DEG,
                     ground_position_llh.y * constant::RAD2DEG, ground_position_llh.z);

    // enu frame of the receiver
    Float3 east, north, up;
    enu_basis_from_llh(ground_position_llh, east, north, up);

    // the receiver antenna basis
    auto rx = east;
    auto ry = north;
    auto rz = cross_product(rx, ry);
    if (!rz.normalize()) return false;

    VERBOSEF("rx:  %+.4f, %+.4f, %+.4f", rx.x, rx.y, rx.z);
    VERBOSEF("ry:  %+.4f, %+.4f, %+.4f", ry.x, ry.y, ry.z);
    VERBOSEF("rz:  %+.4f, %+.4f, %+.4f", rz.x, rz.y, rz.z);

    x = rx;
    y = ry;
    z = rz;
    return true;
}

void Observation::compute_phase_windup() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

#if 1
    Float3 sun{};
    Float3 moon{};
    if (!compute_sun_and_moon_position_ecef(mCurrent->reception_time, sun, moon, nullptr)) {
        VERBOSEF("failed to compute sun and moon position");
        return;
    }

    Float3 sx, sy, sz;
    if (!compute_satellite_antenna_basis_sun(mCurrent->true_position, sun, sx, sy, sz)) {
        VERBOSEF("failed to compute satellite antenna basis");
        return;
    }
#elif 0
    Float3 sx, sy, sz;
    if (!compute_satellite_antenna_basis_velocity(mGroundPosition, mCurrent->true_position,
                                                  mCurrent->true_velocity, sx, sy, sz)) {
        VERBOSEF("failed to compute satellite antenna basis");
        return;
    }
#else
    Float3 sx, sy, sz;
    if (!compute_satellite_antenna_basis_yaw(mCurrent->emission_time, mGroundPosition,
                                             mCurrent->true_position, mCurrent->true_velocity, sx,
                                             sy, sz)) {
        VERBOSEF("failed to compute satellite antenna basis");
        return;
    }
#endif

    Float3 rx, ry, rz;
    if (!compute_receiver_antenna_basis(mGroundLlh, rx, ry, rz)) {
        VERBOSEF("failed to compute receiver antenna basis");
        return;
    }

#if 1
    auto k = mCurrent->true_line_of_sight;  // mGroundPosition - mCurrent->true_position;
    if (!k.normalize()) return;
    VERBOSEF("k:   %+.4f, %+.4f, %+.4f", k.x, k.y, k.z);

    auto sd = sx - k * dot_product(k, sx) - cross_product(k, sy);
    auto rd = rx - k * dot_product(k, rx) + cross_product(k, ry);
    VERBOSEF("sd:  %+.4f, %+.4f, %+.4f", sd.x, sd.y, sd.z);
    VERBOSEF("rd:  %+.4f, %+.4f, %+.4f", rd.x, rd.y, rd.z);

    auto zeta = dot_product(cross_product(sd, rd), k);
    VERBOSEF("zeta: %+.4f", zeta);

    auto cosp = dot_product(sd, rd) / (rd.length() * sd.length());
    if (cosp < -1.0) cosp = -1.0;
    if (cosp > 1.0) cosp = 1.0;
    VERBOSEF("cos: %+.4f", cosp);
    auto ph = acos(cosp) / (2.0 * constant::PI);
    VERBOSEF("ph:  %+.4f", ph);
    if (zeta < 0.0) ph = -ph;

    auto prev_phw = 0.0;
    VERBOSEF("phw: %+.4f", prev_phw);
    auto phw = ph + floor(prev_phw - ph + 0.5);
    VERBOSEF("phw: %+.4f (%+.4f)", phw, ph);

    printf("TRACK-PHW,%s,%s,%.14f,%.14f,%.14f,%.14f,%.14f,%.14f,%.14f,%.14f,%.14f,,%.14f,%.14f,%."
           "14f,%.14f,%.14f,%.14f,%.14f,%.14f,%.14f,,%.14f,%.14f,%.14f,%.14f,%.14f\n",
           mSvId.name(), mSignalId.name(),  //
           rx.x, rx.y, rx.z,                //
           ry.x, ry.y, ry.z,                //
           rd.x, rd.y, rd.z,                //
           sx.x, sx.y, sx.z,                //
           sy.x, sy.y, sy.z,                //
           sd.x, sd.y, sd.z,                //
           k.x, k.y, k.z,                   //
           ph, phw);
#else

#endif

    mPhaseWindup.correction = phw;
    mPhaseWindup.valid      = true;
}

void Observation::compute_antenna_phase_variation() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    mAntennaPhaseVariation.valid = false;
}

void Observation::compute_ranges() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    VERBOSEF("true_range:   %+24.10f", mCurrent->true_range);

    auto clock_bias = constant::SPEED_OF_LIGHT * -mCurrent->eph_clock_bias;
    VERBOSEF("clock_bias:   %+24.10f (%gs)", clock_bias, -mCurrent->eph_clock_bias);

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
        VERBOSEF("stec_grid:    %+24.10f (%gTECU, %gkHz)", stec_grid, mIonospheric.grid_residual,
                 mFrequency);
        VERBOSEF("stec_poly:    %+24.10f (%gTECU, %gkHz)", stec_poly, mIonospheric.poly_residual,
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
        phase_windup = -mPhaseWindup.correction * mWavelength;
        VERBOSEF("phase_windup: %+24.10f (%gc x %gm)", phase_windup, mPhaseWindup.correction,
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
    auto code_result = mCurrent->true_range + clock_bias + code_correction + final_code_correction;
    auto phase_result =
        mCurrent->true_range + clock_bias + phase_correction + final_phase_correction;
    DEBUGF("%s(%s): code  %+24.10f (%g, %g)", mSvId.name(), mSignalId.name(), code_result,
           code_correction, final_code_correction);
    DEBUGF("%s(%s): phase %+24.10f (%g, %g)", mSvId.name(), mSignalId.name(), phase_result,
           phase_correction, final_phase_correction);

    auto next_phase_result = mNext->true_range + constant::SPEED_OF_LIGHT * -mNext->eph_clock_bias +
                             phase_correction + final_phase_correction;
    auto phase_delta = next_phase_result - phase_result;
    auto time_delta =
        (mNext->reception_time.timestamp() - mCurrent->reception_time.timestamp()).full_seconds();
    auto phase_rate = phase_delta / time_delta;

    DEBUGF("%s(%s): phase rate %+19.10f (%g, %g)", mSvId.name(), mSignalId.name(), phase_rate,
           phase_delta, time_delta);

    printf("TRACK-OBS,%s,%s,%g,%u,%.14f,%.14f,%.14f,%.14f,%.14f,%.14f,%.14f,TOTAL,%.14f,%.14f,"
           "TOTAL,%.14f,%."
           "14f,%.14f,%.14f\n",
           mSvId.name(), mSignalId.name(), mFrequency / 1.0e6, mCurrent->eph_iode,
           mCurrent->true_range, clock_bias, clock, code_bias, phase_bias, stec_grid, stec_poly,
           tropo_dry, tropo_wet, shapiro, solid_tides, phase_windup, antenna_phase_variation);

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
