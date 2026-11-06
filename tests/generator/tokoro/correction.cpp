#include <doctest/doctest.h>

#include <cmath>
#include <cstring>
#include <vector>

#include <data/correction.hpp>
#include <data/ionosphere.hpp>
#include <data/set.hpp>
#include <data/status.hpp>
#include <data/troposphere.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <GNSS-SSR-ClockCorrections-r15.h>
#include <GNSS-SSR-CodeBias-r15.h>
#include <GNSS-SSR-GriddedCorrection-r16.h>
#include <GNSS-SSR-OrbitCorrections-r15.h>
#include <GNSS-SSR-STEC-Correction-r16.h>
#include <GridElement-r16.h>
#include <SSR-ClockCorrectionSatelliteElement-r15.h>
#include <SSR-OrbitCorrectionSatelliteElement-r15.h>
#include <STEC-ResidualSatElement-r16.h>
#include <STEC-ResidualSatList-r16.h>
#include <STEC-SatElement-r16.h>
#include <TropospericDelayCorrection-r16.h>
EXTERNAL_WARNINGS_POP

using namespace generator::tokoro;

static constexpr double DEG2RAD = M_PI / 180.0;

static uint64_t all_points(long count) {
    uint64_t mask = 0;
    for (long i = 0; i < count; i++)
        mask |= 1ULL << (63 - i);
    return mask;
}

// 2x2 array (one cell), north-west corner at 60N 10E, 1 degree spacing
static CorrectionPointSet make_set(uint16_t id = 7, double reference_latitude = 60.0) {
    CorrectionPointSet set{};
    set.set_id                    = id;
    set.grid_point_count          = 4;
    set.reference_point_latitude  = reference_latitude;
    set.reference_point_longitude = 10.0;
    set.step_of_latitude          = 1.0;
    set.step_of_longitude         = 1.0;
    set.number_of_steps_latitude  = 1;
    set.number_of_steps_longitude = 1;
    set.bitmask                   = all_points(4);
    return set;
}

static void set_time(GNSS_SystemTime& time, long time_of_day) {
    time.gnss_TimeID.gnss_id = GNSS_ID__gnss_id_gps;
    time.gnss_DayNumber      = 16000;
    time.gnss_TimeOfDay      = time_of_day;
}

static Float3 llh(double lat, double lon) {
    return Float3{lat * DEG2RAD, lon * DEG2RAD, 0.0};
}

static SatelliteId const G01 = SatelliteId::from_gps_prn(1);
static SatelliteId const G02 = SatelliteId::from_gps_prn(2);

namespace {

struct Stec {
    GNSS_SSR_STEC_Correction_r16 msg{};
    STEC_SatElement_r16          element{};
    STEC_SatElement_r16*         pointer{&element};
    uint8_t                      quality{0};

    // C00 raw 100 -> 5 TECU
    explicit Stec(long time_of_day) {
        set_time(msg.epochTime_r16, time_of_day);
        msg.correctionPointSetID_r16          = 7;
        element.svID_r16.satellite_id         = 0;
        element.stecQualityIndicator_r16.buf  = &quality;
        element.stecQualityIndicator_r16.size = 1;
        element.stec_C00_r16                  = 100;
        msg.stec_SatList_r16.list.array       = &pointer;
        msg.stec_SatList_r16.list.count       = 1;
        msg.stec_SatList_r16.list.size        = 1;
    }
};

struct Gridded {
    GNSS_SSR_GriddedCorrection_r16              msg{};
    std::vector<GridElement_r16>                elements;
    std::vector<STEC_ResidualSatList_r16>       residual_lists;
    std::vector<STEC_ResidualSatElement_r16>    residuals;
    std::vector<STEC_ResidualSatElement_r16*>   residual_pointers;
    std::vector<TropospericDelayCorrection_r16> tropo;
    std::vector<GridElement_r16*>               pointers;

    // `satellites` LPP ids with a residual of raw 25 (1 TECU) at every point
    Gridded(long time_of_day, std::vector<long> const& satellites, bool with_tropo)
        : elements(4), residual_lists(4), residuals(4 * satellites.size()),
          residual_pointers(4 * satellites.size()), tropo(4), pointers(4) {
        set_time(msg.epochTime_r16, time_of_day);
        msg.correctionPointSetID_r16 = 7;
        for (size_t j = 0; j < 4; j++) {
            elements[j] = {};
            if (with_tropo) {
                tropo[j]                                   = {};
                elements[j].tropospericDelayCorrection_r16 = &tropo[j];
            }
            if (!satellites.empty()) {
                for (size_t s = 0; s < satellites.size(); s++) {
                    auto& residual                 = residuals[j * satellites.size() + s];
                    residual                       = {};
                    residual.svID_r16.satellite_id = satellites[s];
                    residual.stecResidualCorrection_r16.present =
                        STEC_ResidualSatElement_r16__stecResidualCorrection_r16_PR_b7_r16;
                    residual.stecResidualCorrection_r16.choice.b7_r16 = 25;
                    residual_pointers[j * satellites.size() + s]      = &residual;
                }
                residual_lists[j]                    = {};
                residual_lists[j].list.array         = &residual_pointers[j * satellites.size()];
                residual_lists[j].list.count         = static_cast<int>(satellites.size());
                residual_lists[j].list.size          = static_cast<int>(satellites.size());
                elements[j].stec_ResidualSatList_r16 = &residual_lists[j];
            }
            pointers[j] = &elements[j];
        }
        msg.gridList_r16.list.array = pointers.data();
        msg.gridList_r16.list.count = 4;
        msg.gridList_r16.list.size  = 4;
    }
};

}  // namespace

TEST_CASE("Tokoro iono: grid residual is required by default") {
    auto           set = make_set();
    CorrectionData data{};
    data.correction_point_set = &set;

    Stec stec(3600);
    data.add_correction(GNSS_ID__gnss_id_gps, &stec.msg, set);

    IonosphericCorrection iono{};
    DataStatus            status;
    CHECK_FALSE(data.ionospheric(G01, llh(59.5, 10.5), iono, &status));
    CHECK(std::string(status.text()) == "no_grid");

    data.iono_polynomial_only = true;
    status                    = {};
    REQUIRE(data.ionospheric(G01, llh(59.5, 10.5), iono, &status));
    CHECK(status.ok());
    CHECK_FALSE(iono.grid_valid);
    CHECK(iono.polynomial_residual == doctest::Approx(5.0));
    CHECK(iono.grid_residual == 0.0);

    data.iono_polynomial_only = false;
    Gridded gridded(3600, {0}, true);
    data.add_correction(GNSS_ID__gnss_id_gps, &gridded.msg, set);

    status = {};
    REQUIRE(data.ionospheric(G01, llh(59.5, 10.5), iono, &status));
    CHECK(iono.grid_valid);
    CHECK(iono.grid_residual == doctest::Approx(1.0));

    // outside the grid
    status = {};
    CHECK_FALSE(data.ionospheric(G01, llh(61.0, 10.5), iono, &status));
    CHECK(std::string(status.text()) == "grid_outside");
}

TEST_CASE("Tokoro iono: satellite missing from a later grid message") {
    auto           set = make_set();
    CorrectionData data{};
    data.correction_point_set = &set;

    Stec stec(3600);
    data.add_correction(GNSS_ID__gnss_id_gps, &stec.msg, set);

    Gridded first(3600, {0, 1}, true);
    data.add_correction(GNSS_ID__gnss_id_gps, &first.msg, set);

    IonosphericCorrection iono{};
    REQUIRE(data.ionospheric(G01, llh(59.5, 10.5), iono));

    // G01 (LPP id 0) is no longer provided
    Gridded second(3605, {1}, true);
    data.add_correction(GNSS_ID__gnss_id_gps, &second.msg, set);

    DataStatus status;
    CHECK_FALSE(data.ionospheric(G01, llh(59.5, 10.5), iono, &status));
    CHECK(std::string(status.text()) == "grid_missing_sat");

    // a message with only troposphere keeps the residuals of the other satellites
    Gridded tropo_only(3610, {}, true);
    data.add_correction(GNSS_ID__gnss_id_gps, &tropo_only.msg, set);
    TroposphericCorrection tropo{};
    CHECK(data.tropospheric(G02, llh(59.5, 10.5), tropo));
}

TEST_CASE("Tokoro grid: rebuilt when the layout changes under the same id") {
    auto           set = make_set(7, 60.0);
    CorrectionData data{};
    data.correction_point_set = &set;

    Gridded gridded(3600, {}, true);
    data.add_correction(GNSS_ID__gnss_id_gps, &gridded.msg, set);

    TroposphericCorrection tropo{};
    CHECK(data.tropospheric(G01, llh(59.5, 10.5), tropo));
    CHECK_FALSE(data.tropospheric(G01, llh(50.5, 10.5), tropo));

    auto moved                = make_set(7, 51.0);
    data.correction_point_set = &moved;
    data.add_correction(GNSS_ID__gnss_id_gps, &gridded.msg, moved);

    CHECK(data.tropospheric(G01, llh(50.5, 10.5), tropo));
    CHECK_FALSE(data.tropospheric(G01, llh(59.5, 10.5), tropo));
}

namespace {

struct Orbit {
    GNSS_SSR_OrbitCorrections_r15            msg{};
    SSR_OrbitCorrectionSatelliteElement_r15  element{};
    SSR_OrbitCorrectionSatelliteElement_r15* pointer{&element};
    uint8_t                                  iod[2]{0, 0};

    Orbit(long time_of_day, long update_interval) {
        set_time(msg.epochTime_r15, time_of_day);
        msg.ssrUpdateInterval_r15                  = update_interval;
        element.svID_r15.satellite_id              = 0;
        element.iod_r15.buf                        = iod;
        element.iod_r15.size                       = 2;
        element.iod_r15.bits_unused                = 5;
        msg.ssr_OrbitCorrectionList_r15.list.array = &pointer;
        msg.ssr_OrbitCorrectionList_r15.list.count = 1;
        msg.ssr_OrbitCorrectionList_r15.list.size  = 1;
    }
};

struct Clock {
    GNSS_SSR_ClockCorrections_r15            msg{};
    SSR_ClockCorrectionSatelliteElement_r15  element{};
    SSR_ClockCorrectionSatelliteElement_r15* pointer{&element};

    Clock(long time_of_day, long update_interval) {
        set_time(msg.epochTime_r15, time_of_day);
        msg.ssrUpdateInterval_r15                  = update_interval;
        element.svID_r15.satellite_id              = 1;
        msg.ssr_ClockCorrectionList_r15.list.array = &pointer;
        msg.ssr_ClockCorrectionList_r15.list.count = 1;
        msg.ssr_ClockCorrectionList_r15.list.size  = 1;
    }
};

}  // namespace

TEST_CASE("Tokoro orbit/clock: age is measured from the SSR epoch") {
    CorrectionData data{};
    data.max_orbit_age = 30.0;
    data.max_clock_age = 30.0;

    // update interval 6 = 60 s, so the reference time t0 is epoch + 30 s
    Orbit orbit(3600, 6);
    Clock clock(3600, 6);
    data.add_correction(GNSS_ID__gnss_id_gps, &orbit.msg);
    data.add_correction(GNSS_ID__gnss_id_gps, &clock.msg);

    REQUIRE(data.orbit_correction(G01) != nullptr);
    REQUIRE(data.clock_correction(G02) != nullptr);
    CHECK(data.orbit_correction(G01)->reference_time - data.orbit_correction(G01)->epoch_time ==
          doctest::Approx(30.0));

    // 40 s after the epoch (10 s after t0): stale
    Orbit later(3640, 6);
    later.element.svID_r15.satellite_id = 5;
    data.add_correction(GNSS_ID__gnss_id_gps, &later.msg);

    DataStatus status;
    CHECK(data.orbit_correction(G01, &status) == nullptr);
    CHECK(status.has_age);
    CHECK(status.age == doctest::Approx(40.0));
    CHECK(data.clock_correction(G02) == nullptr);
}

#include <generator/tokoro/generator.hpp>
#include <models/frame_transform.hpp>
#include <time/utc.hpp>

TEST_CASE("Tokoro broadcast orbit frames") {
    CHECK(OrbitFrames::create("not-a-frame") == nullptr);

    auto frames = OrbitFrames::create("itrf2020");
    REQUIRE(frames);
    CHECK(std::string(frames->target_name()) == "itrf2020");

    auto   time = ts::Tai(ts::Utc::from_date_time(2026, 6, 6, 0, 15, 0.0));
    Float3 original{10732367.1875, -11441130.37109, 19524788.08594};
    Float3 velocity{-2112.96, -1364.08, 233.65};

    // GLONASS: PZ-90.11 -> ITRF2008 -> ITRF2020
    CHECK(std::string(frames->source_name(SatelliteId::GLONASS, time)) == "pz-90.11");
    auto position = original;
    REQUIRE(frames->apply(SatelliteId::GLONASS, time, position, velocity));
    auto difference = (position - original).length();
    CHECK(difference > 0.001);
    CHECK(difference < 0.03);

    // GPS: WGS 84 (G2296), aligned with ITRF2020
    CHECK(std::string(frames->source_name(SatelliteId::GPS, time)) == "wgs84(g2296)");
    position = original;
    REQUIRE(frames->apply(SatelliteId::GPS, time, position, velocity));
    CHECK((position - original).length() < 1e-9);

    // Galileo: not transformed unless a source frame is set
    CHECK(frames->source_name(SatelliteId::GALILEO, time) == nullptr);
    CHECK_FALSE(frames->apply(SatelliteId::GALILEO, time, position, velocity));
    CHECK_FALSE(frames->set_source(SatelliteId::GALILEO, "not-a-frame"));
    REQUIRE(frames->set_source(SatelliteId::GALILEO, "itrf2014"));
    position = original;
    REQUIRE(frames->apply(SatelliteId::GALILEO, time, position, velocity));
    CHECK((position - original).length() > 0.0);
    CHECK((position - original).length() < 0.05);

    SatelliteId::Gnss gnss{};
    CHECK(gnss_from_name("bds", gnss));
    CHECK(gnss == SatelliteId::BEIDOU);
    CHECK_FALSE(gnss_from_name("xyz", gnss));
}

TEST_CASE("Tokoro ground position frame") {
    // Linköping, Sweden
    Float3 position{3227560.2402, 898383.8750, 5409177.4433};
    auto   time = ts::Tai(ts::Utc::from_date_time(2026, 6, 6, 0, 0, 0.0));
    auto   year = decimal_year(time);
    CHECK(year == doctest::Approx(2026.43).epsilon(1e-4));

    CHECK(GroundFrame::create("not-a-frame", 0.0, {}, "itrf2020") == nullptr);
    CHECK(GroundFrame::create("itrf2020", 0.0, {}, "not-a-frame") == nullptr);

    // same frame, no velocity: unchanged
    auto same = GroundFrame::create("itrf2020", 0.0, {}, "itrf2020");
    REQUIRE(same);
    CHECK((same->position(position, time) - position).length() < 1e-9);

    // same frame, moved with the station velocity from 2020.0
    Float3 velocity{-0.015, 0.016, 0.010};
    auto   moved = GroundFrame::create("itrf2020", 2020.0, velocity, "itrf2020");
    REQUIRE(moved);
    auto p = moved->position(position, time);
    CHECK(p.x - position.x == doctest::Approx(velocity.x * (year - 2020.0)));
    CHECK(p.z - position.z == doctest::Approx(velocity.z * (year - 2020.0)));

    // SWEREF 99 (ETRF89 at 1999.5) to ITRF2020: about 2.5 cm/year of plate motion since 1999.5
    auto sweref = GroundFrame::create("sweref99", 0.0, {}, "itrf2020");
    REQUIRE(sweref);
    auto difference = (sweref->position(position, time) - position).length();
    CHECK(difference > 0.5);
    CHECK(difference < 1.0);

    // ETRF2014 to ITRF2020 at the observation epoch
    auto etrf = GroundFrame::create("etrf2014", 0.0, {}, "itrf2020");
    REQUIRE(etrf);
    difference = (etrf->position(position, time) - position).length();
    CHECK(difference > 0.5);
    CHECK(difference < 1.0);
}
