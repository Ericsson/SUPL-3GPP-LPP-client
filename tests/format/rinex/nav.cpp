#include <doctest/doctest.h>
#include <format/rinex/nav.hpp>
#include <gnss/satellite_id.hpp>
#include <time/gps.hpp>

#include <cstdio>
#include <cstring>
#include <unistd.h>

static std::string write_tmp(char const* content) {
    char path[] = "/tmp/rinex_nav_test_XXXXXX";
    int  fd     = mkstemp(path);
    REQUIRE(fd >= 0);
    write(fd, content, strlen(content));
    close(fd);
    return path;
}

// Minimal GPS nav file with one satellite
static char const* GPS_NAV =
    R"(     3.04           N: GNSS NAV DATA    G: GPS              RINEX VERSION / TYPE
                                                            END OF HEADER
G04 2026 03 17 00 00 00  .316237565130E-03 -.682121026330E-11  .000000000000E+00
      .150000000000E+02  .252500000000E+02  .435982446133E-08 -.145595300048E+01
      .122562050819E-05  .162061932497E-02  .130590051413E-04  .515362702942E+04
      .172800000000E+06  .130385160446E-07  .288666004331E+01 -.316649675369E-07
      .958182090873E+00  .125937500000E+03  .108317730063E+00 -.790211486909E-08
     -.767889128521E-10  .000000000000E+00  .241000000000E+04  .000000000000E+00
      .200000000000E+01  .000000000000E+00 -.931322574615E-08  .150000000000E+02
      .165618000000E+06  .000000000000E+00  .000000000000E+00  .000000000000E+00
)";

TEST_CASE("RINEX nav - GPS satellite ID") {
    auto path = write_tmp(GPS_NAV);

    std::vector<ephemeris::GpsEphemeris> ephs;
    format::rinex::NavCallbacks          cb;
    cb.gps = [&](ephemeris::GpsEphemeris const& e) {
        ephs.push_back(e);
    };
    bool ok = format::rinex::parse_nav(path, cb);
    unlink(path.c_str());

    CHECK(ok);
    REQUIRE(ephs.size() == 1);
    CHECK(ephs[0].prn == 4);
    CHECK(SatelliteId::from_gps_prn(ephs[0].prn).name() == std::string("G04"));
}

TEST_CASE("RINEX nav - GPS clock parameters") {
    auto path = write_tmp(GPS_NAV);

    std::vector<ephemeris::GpsEphemeris> ephs;
    format::rinex::NavCallbacks          cb;
    cb.gps = [&](ephemeris::GpsEphemeris const& e) {
        ephs.push_back(e);
    };
    format::rinex::parse_nav(path, cb);
    unlink(path.c_str());

    REQUIRE(ephs.size() == 1);
    CHECK(ephs[0].af0 == doctest::Approx(3.16237565130e-04));
    CHECK(ephs[0].af1 == doctest::Approx(-6.82121026330e-12));
    CHECK(ephs[0].af2 == doctest::Approx(0.0));
}

TEST_CASE("RINEX nav - GPS orbital parameters") {
    auto path = write_tmp(GPS_NAV);

    std::vector<ephemeris::GpsEphemeris> ephs;
    format::rinex::NavCallbacks          cb;
    cb.gps = [&](ephemeris::GpsEphemeris const& e) {
        ephs.push_back(e);
    };
    format::rinex::parse_nav(path, cb);
    unlink(path.c_str());

    REQUIRE(ephs.size() == 1);
    // a = sqrt_a^2 where sqrt_a = 5153.62702942
    double sqrt_a = 5153.62702942;
    CHECK(ephs[0].a == doctest::Approx(sqrt_a * sqrt_a));
    CHECK(ephs[0].e == doctest::Approx(1.62061932497e-03));  // .162061932497E-02
    CHECK(ephs[0].i0 == doctest::Approx(9.58182090873e-01));
}

// Actual G04 and G05 from 1LIN RINEX nav file (2026-03-17 00:00:00 GPS)
static char const* LIN_NAV_G04G05 =
    R"(     3.04           N: GNSS NAV DATA    G: GPS              RINEX VERSION / TYPE
                                                            END OF HEADER
G04 2026 03 17 00 00 00  .491407699883E-04  .181898940355E-11  .000000000000E+00
      .540000000000E+02 -.819687500000E+02  .457340478638E-08 -.597182823039E+00
     -.442378222942E-05  .373577128630E-02  .353343784809E-05  .515372620964E+04
      .172800000000E+06  .134110450745E-06 -.133886342490E+01  .316649675369E-07
      .970454185872E+00  .316531250000E+03 -.287422325884E+01 -.836963434315E-08
     -.468590947265E-09  .100000000000E+01  .241000000000E+04  .000000000000E+00
      .200000000000E+01  .000000000000E+00 -.465661287308E-08  .822000000000E+03
      .165618000000E+06  .000000000000E+00  .000000000000E+00  .000000000000E+00
G05 2026 03 17 00 00 00 -.230424571782E-03 -.568434188608E-13  .000000000000E+00
      .820000000000E+02  .957812500000E+02  .437446792843E-08  .721205505333E-01
      .523403286934E-05  .542650534771E-02  .586360692978E-05  .515371077156E+04
      .172800000000E+06  .000000000000E+00 -.246936237154E+01  .260770320892E-07
      .980072955630E+00  .276500000000E+03  .141774131546E+01 -.822748556494E-08
      .335728270144E-09  .000000000000E+00  .241000000000E+04  .000000000000E+00
      .200000000000E+01  .000000000000E+00 -.107102096081E-07  .820000000000E+02
      .165618000000E+06  .000000000000E+00  .000000000000E+00  .000000000000E+00
)";

// Parse helper
static std::vector<ephemeris::GpsEphemeris> parse_lin(char const* content) {
    auto                                 path = write_tmp(content);
    std::vector<ephemeris::GpsEphemeris> ephs;
    format::rinex::NavCallbacks          cb;
    cb.gps = [&](ephemeris::GpsEphemeris const& e) {
        ephs.push_back(e);
    };
    format::rinex::parse_nav(path, cb);
    unlink(path.c_str());
    return ephs;
}

TEST_CASE("RINEX nav - 1LIN G04 orbital parameters") {
    auto ephs = parse_lin(LIN_NAV_G04G05);
    REQUIRE(ephs.size() == 2);
    auto& e = ephs[0];
    CHECK(e.prn == 4);
    CHECK(e.af0 == doctest::Approx(4.91407699883e-05));
    CHECK(e.af1 == doctest::Approx(1.81898940355e-11));
    CHECK(e.crs == doctest::Approx(-81.96875));
    CHECK(e.delta_n == doctest::Approx(4.57340478638e-09));
    CHECK(e.m0 == doctest::Approx(-5.97182823039e-01));
    CHECK(e.cuc == doctest::Approx(-4.42378222942e-06));
    CHECK(e.e == doctest::Approx(3.73577128630e-03));
    CHECK(e.cus == doctest::Approx(3.53343784809e-06));
    double sqrt_a = 5153.72620964;
    CHECK(e.a == doctest::Approx(sqrt_a * sqrt_a));
    CHECK(e.toe == doctest::Approx(172800.0));
    CHECK(e.cic == doctest::Approx(1.34110450745e-07));
    CHECK(e.omega0 == doctest::Approx(-1.33886342490e+00));
    CHECK(e.cis == doctest::Approx(3.16649675369e-08));
    CHECK(e.i0 == doctest::Approx(9.70454185872e-01));
    CHECK(e.crc == doctest::Approx(316.53125));
    CHECK(e.omega == doctest::Approx(-2.87422325884e+00));
    CHECK(e.omega_dot == doctest::Approx(-8.36963434315e-09));
    CHECK(e.idot == doctest::Approx(-4.68590947265e-10));
}

TEST_CASE("RINEX nav - 1LIN G05 orbital parameters") {
    auto ephs = parse_lin(LIN_NAV_G04G05);
    REQUIRE(ephs.size() == 2);
    auto& e = ephs[1];
    CHECK(e.prn == 5);
    CHECK(e.af0 == doctest::Approx(-2.30424571782e-04));
    CHECK(e.af1 == doctest::Approx(-5.68434188608e-13));
    CHECK(e.crs == doctest::Approx(95.78125));
    CHECK(e.delta_n == doctest::Approx(4.37446792843e-09));
    CHECK(e.m0 == doctest::Approx(7.21205505333e-02));
    CHECK(e.e == doctest::Approx(5.42650534771e-03));
    double sqrt_a = 5153.71077156;
    CHECK(e.a == doctest::Approx(sqrt_a * sqrt_a));
    CHECK(e.toe == doctest::Approx(172800.0));
    CHECK(e.omega0 == doctest::Approx(-2.46936237154e+00));
    CHECK(e.i0 == doctest::Approx(9.80072955630e-01));
    CHECK(e.omega == doctest::Approx(1.41774131546e+00));
    CHECK(e.omega_dot == doctest::Approx(-8.22748556494e-09));
    CHECK(e.idot == doctest::Approx(3.35728270144e-10));
}

TEST_CASE("RINEX nav - 1LIN G04 position at TOE") {
    // Reference computed with IS-GPS-200 equations in Python at t_k=0
    // G04 ECEF at TOE: (-100226.267, 25491686.850, 7163450.087)
    auto ephs = parse_lin(LIN_NAV_G04G05);
    REQUIRE(ephs.size() == 2);
    auto& e = ephs[0];
    auto  t = ts::Gps::from_week_tow(2410, 172800, 0.0);
    auto  r = e.compute(t);
    CHECK(r.position.x == doctest::Approx(-100226.267).epsilon(1.0));
    CHECK(r.position.y == doctest::Approx(25491686.850).epsilon(1.0));
    CHECK(r.position.z == doctest::Approx(7163450.087).epsilon(1.0));
    CHECK(r.clock == doctest::Approx(4.91407699883e-05).epsilon(1e-10));
}

TEST_CASE("RINEX nav - 1LIN G05 position at TOE") {
    // Reference computed with IS-GPS-200 equations in Python at t_k=0
    // G05 ECEF at TOE: (7033568.562, -13041868.123, 21869656.795)
    auto ephs = parse_lin(LIN_NAV_G04G05);
    REQUIRE(ephs.size() == 2);
    auto& e = ephs[1];
    auto  t = ts::Gps::from_week_tow(2410, 172800, 0.0);
    auto  r = e.compute(t);
    CHECK(r.position.x == doctest::Approx(7033568.562).epsilon(1.0));
    CHECK(r.position.y == doctest::Approx(-13041868.123).epsilon(1.0));
    CHECK(r.position.z == doctest::Approx(21869656.795).epsilon(1.0));
    CHECK(r.clock == doctest::Approx(-2.30424571782e-04).epsilon(1e-10));
}
