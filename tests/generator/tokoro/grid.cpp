#include <doctest/doctest.h>

#include <cmath>
#include <vector>

#include <data/correction.hpp>
#include <data/set.hpp>
#include <data/troposphere.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <GNSS-SSR-GriddedCorrection-r16.h>
#include <GridElement-r16.h>
#include <TropospericDelayCorrection-r16.h>
EXTERNAL_WARNINGS_POP

using namespace generator::tokoro;

static constexpr double DEG2RAD = M_PI / 180.0;

// 3x3 array, north-west corner at 60N 10E, 1 degree spacing. Absolute index = row * 3 + col.
static CorrectionPointSet make_set(uint64_t bitmask) {
    CorrectionPointSet set{};
    set.set_id                    = 7;
    set.grid_point_count          = 9;
    set.reference_point_latitude  = 60.0;
    set.reference_point_longitude = 10.0;
    set.step_of_latitude          = 1.0;
    set.step_of_longitude         = 1.0;
    set.number_of_steps_latitude  = 2;
    set.number_of_steps_longitude = 2;
    set.bitmask                   = bitmask;
    return set;
}

// bitmaskOfGrids bit for absolute index i, as decoded by the generator (MSB = index 0)
static uint64_t bit(long i) {
    return 1ULL << (63 - i);
}

struct Gridded {
    GNSS_SSR_GriddedCorrection_r16              msg{};
    std::vector<GridElement_r16>                elements;
    std::vector<TropospericDelayCorrection_r16> tropo;
    std::vector<GridElement_r16*>               pointers;

    // Entry j carries hydrostatic raw value j + 1, i.e. 2.3 + 0.004 * (j + 1) m
    explicit Gridded(long count) : elements(count), tropo(count), pointers(count) {
        msg.epochTime_r16.gnss_TimeID.gnss_id = GNSS_ID__gnss_id_gps;
        msg.epochTime_r16.gnss_DayNumber      = 16000;
        msg.epochTime_r16.gnss_TimeOfDay      = 3600;
        msg.correctionPointSetID_r16          = 7;
        for (long j = 0; j < count; j++) {
            tropo[j]                                   = {};
            tropo[j].tropoHydroStaticVerticalDelay_r16 = j + 1;
            tropo[j].tropoWetVerticalDelay_r16         = 0;
            elements[j]                                = {};
            elements[j].tropospericDelayCorrection_r16 = &tropo[j];
            pointers[j]                                = &elements[j];
        }
        msg.gridList_r16.list.array = pointers.data();
        msg.gridList_r16.list.count = static_cast<int>(count);
        msg.gridList_r16.list.size  = static_cast<int>(count);
    }
};

static double dry(long entry) {
    return 2.3 + 0.004 * static_cast<double>(entry + 1);
}

static bool tropo_at(CorrectionData const& data, double lat, double lon, double& value) {
    TroposphericCorrection correction{};
    auto                   ok = data.tropospheric(SatelliteId::from_gps_prn(1),
                                                  Float3{lat * DEG2RAD, lon * DEG2RAD, 0.0}, correction);
    value                     = correction.dry;
    return ok;
}

TEST_CASE("Tokoro grid: masked north-west point") {
    // Point 0 masked: the gridList has 8 entries for absolute indices 1..8
    uint64_t mask = 0;
    for (long i = 1; i < 9; i++)
        mask |= bit(i);
    auto set = make_set(mask);

    Gridded        gridded(8);
    CorrectionData data{};
    data.correction_point_set = &set;
    data.add_correction(GNSS_ID__gnss_id_gps, &gridded.msg, set);

    double value = 0.0;

    // Cell with corners 1,2,4,5 -> entries 0,1,3,4
    REQUIRE(tropo_at(data, 59.5, 11.5, value));
    CHECK(value == doctest::Approx((dry(0) + dry(1) + dry(3) + dry(4)) / 4.0));

    // Cell with corners 3,4,6,7 -> entries 2,3,5,6
    REQUIRE(tropo_at(data, 58.5, 10.5, value));
    CHECK(value == doctest::Approx((dry(2) + dry(3) + dry(5) + dry(6)) / 4.0));

    // Exactly on grid point 8 (south-east corner) -> entry 7
    REQUIRE(tropo_at(data, 58.0, 12.0, value));
    CHECK(value == doctest::Approx(dry(7)));

    // On the edge between the incomplete cell 0,1,3,4 and the complete cell 3,4,6,7
    REQUIRE(tropo_at(data, 59.0, 10.5, value));
    CHECK(value == doctest::Approx((dry(2) + dry(3)) / 2.0));

    // Cell with the masked corner 0 has no correction
    CHECK_FALSE(tropo_at(data, 59.5, 10.5, value));
}

TEST_CASE("Tokoro grid: masked north-east point") {
    // Point 2 masked: entries 0,1 -> indices 0,1; entries 2..7 -> indices 3..8
    uint64_t mask = 0;
    for (long i = 0; i < 9; i++)
        if (i != 2) mask |= bit(i);
    auto set = make_set(mask);

    Gridded        gridded(8);
    CorrectionData data{};
    data.correction_point_set = &set;
    data.add_correction(GNSS_ID__gnss_id_gps, &gridded.msg, set);

    double value = 0.0;

    // Cell 0,1,3,4 -> entries 0,1,2,3
    REQUIRE(tropo_at(data, 59.5, 10.5, value));
    CHECK(value == doctest::Approx((dry(0) + dry(1) + dry(2) + dry(3)) / 4.0));

    // Cell 4,5,7,8 -> entries 3,4,6,7
    REQUIRE(tropo_at(data, 58.5, 11.5, value));
    CHECK(value == doctest::Approx((dry(3) + dry(4) + dry(6) + dry(7)) / 4.0));

    // Cell 1,2,4,5 contains the masked corner
    CHECK_FALSE(tropo_at(data, 59.5, 11.5, value));
}

TEST_CASE("Tokoro grid: no bitmask") {
    auto set = make_set(0xFFFFFFFFFFFFFFFFULL);

    Gridded        gridded(9);
    CorrectionData data{};
    data.correction_point_set = &set;
    data.add_correction(GNSS_ID__gnss_id_gps, &gridded.msg, set);

    double value = 0.0;
    REQUIRE(tropo_at(data, 59.5, 10.5, value));
    CHECK(value == doctest::Approx((dry(0) + dry(1) + dry(3) + dry(4)) / 4.0));
    REQUIRE(tropo_at(data, 58.0, 12.0, value));
    CHECK(value == doctest::Approx(dry(8)));
    REQUIRE(tropo_at(data, 60.0, 10.0, value));
    CHECK(value == doctest::Approx(dry(0)));

    // Outside the grid
    CHECK_FALSE(tropo_at(data, 60.1, 10.5, value));
    CHECK_FALSE(tropo_at(data, 59.5, 12.1, value));
}

TEST_CASE("Tokoro grid: entry count does not match bitmask") {
    uint64_t mask = 0;
    for (long i = 1; i < 9; i++)
        mask |= bit(i);
    auto set = make_set(mask);

    Gridded        gridded(9);
    CorrectionData data{};
    data.correction_point_set = &set;
    data.add_correction(GNSS_ID__gnss_id_gps, &gridded.msg, set);

    double value = 0.0;
    CHECK_FALSE(tropo_at(data, 58.5, 11.5, value));
}
