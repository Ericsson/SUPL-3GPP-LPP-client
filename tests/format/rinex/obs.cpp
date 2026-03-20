#include <doctest/doctest.h>
#include <format/rinex/obs.hpp>
#include <time/gps.hpp>
#include <time/utc.hpp>

#include <cstdio>
#include <cstring>
#include <unistd.h>

// Write content to a temp file, return path (caller must unlink)
static std::string write_tmp(char const* content) {
    char path[] = "/tmp/rinex_obs_test_XXXXXX";
    int  fd     = mkstemp(path);
    REQUIRE(fd >= 0);
    write(fd, content, strlen(content));
    close(fd);
    return path;
}

// Minimal RINEX 3 obs file with one GPS epoch, two satellites
static char const* SIMPLE_OBS =
    R"(     3.04           OBSERVATION DATA    G (GPS)             RINEX VERSION / TYPE
                                                            END OF HEADER
> 2026 03 17  0  0  0.0000000  0  2
G04  24939143.384   34.000
G07  21103992.340   51.000
)";

// RINEX 3 obs with proper SYS / # / OBS TYPES header
static char const* OBS_WITH_TYPES =
    R"(     3.04           OBSERVATION DATA    M (MIXED)           RINEX VERSION / TYPE
G    4 C1C L1C S1C C2W                                      SYS / # / OBS TYPES
E    2 C1C L1C                                              SYS / # / OBS TYPES
                                                            END OF HEADER
> 2026 03 17  0  0  0.0000000  0  2
G04  24939143.384  131056122.808        34.000  24939141.668
E11  23456789.012  123456789.012
)";

// RINEX 3 obs with 18 GPS obs types (requires continuation line)
// C1C at i=0, C5Q at i=15, rest blank — properly column-aligned (16 chars/field)
static char const* OBS_CONTINUATION =
    R"(     3.04           OBSERVATION DATA    G (GPS)             RINEX VERSION / TYPE
G   18 C1C C1L L1C L1L S1C S1L C2P C2W C2L L2P L2W L2L S2P  SYS / # / OBS TYPES
       S2W S2L C5Q L5Q S5Q                                  SYS / # / OBS TYPES
                                                            END OF HEADER
> 2026 03 17  0  0  0.0000000  0  1
G04  24939143.384                                                                                                                                                                                                                                    24939145.045                                  
)";

TEST_CASE("RINEX obs - parse epoch time") {
    auto path = write_tmp(OBS_WITH_TYPES);

    std::vector<format::rinex::ObsEpoch> epochs;
    format::rinex::parse_obs(path, [&](format::rinex::ObsEpoch const& e) {
        epochs.push_back(e);
    });
    unlink(path.c_str());

    REQUIRE(epochs.size() == 1);
    // 2026-03-17 00:00:00 GPS time → TAI
    auto gps = ts::Gps::from_ymdhms(2026, 3, 17, 0, 0, 0.0);
    CHECK(epochs[0].time == ts::Tai{gps});
}

TEST_CASE("RINEX obs - satellite IDs") {
    auto path = write_tmp(OBS_WITH_TYPES);

    std::vector<format::rinex::ObsEpoch> epochs;
    format::rinex::parse_obs(path, [&](format::rinex::ObsEpoch const& e) {
        epochs.push_back(e);
    });
    unlink(path.c_str());

    REQUIRE(epochs.size() == 1);
    // Collect satellite IDs
    std::vector<std::string> svs;
    for (auto const& m : epochs[0].measurements)
        svs.push_back(m.satellite_id.name());

    // G04 and E11 should appear
    bool has_g04 = false, has_e11 = false;
    for (auto const& s : svs) {
        if (s == "G04") has_g04 = true;
        if (s == "E11") has_e11 = true;
    }
    CHECK(has_g04);
    CHECK(has_e11);
}

TEST_CASE("RINEX obs - pseudorange value") {
    auto path = write_tmp(OBS_WITH_TYPES);

    std::vector<format::rinex::ObsEpoch> epochs;
    format::rinex::parse_obs(path, [&](format::rinex::ObsEpoch const& e) {
        epochs.push_back(e);
    });
    unlink(path.c_str());

    REQUIRE(epochs.size() == 1);
    double g04_c1c = 0.0;
    for (auto const& m : epochs[0].measurements) {
        if (std::string(m.satellite_id.name()) == "G04" && m.signal_id == SignalId::GPS_L1_CA)
            g04_c1c = m.pseudorange;
    }
    CHECK(g04_c1c == doctest::Approx(24939143.384));
}

TEST_CASE("RINEX obs - SNR from S-type observation") {
    auto path = write_tmp(OBS_WITH_TYPES);

    std::vector<format::rinex::ObsEpoch> epochs;
    format::rinex::parse_obs(path, [&](format::rinex::ObsEpoch const& e) {
        epochs.push_back(e);
    });
    unlink(path.c_str());

    REQUIRE(epochs.size() == 1);
    double snr = 0.0;
    for (auto const& m : epochs[0].measurements) {
        if (std::string(m.satellite_id.name()) == "G04" && m.signal_id == SignalId::GPS_L1_CA)
            snr = m.snr;
    }
    CHECK(snr == doctest::Approx(34.0));
}

TEST_CASE("RINEX obs - obs types continuation line") {
    auto path = write_tmp(OBS_CONTINUATION);

    std::vector<format::rinex::ObsEpoch> epochs;
    format::rinex::parse_obs(path, [&](format::rinex::ObsEpoch const& e) {
        epochs.push_back(e);
    });
    unlink(path.c_str());

    REQUIRE(epochs.size() == 1);
    // Should have GPS_L1_CA (C1C) and GPS_L5_Q (C5Q) measurements for G04
    bool has_l1ca = false, has_l5q = false;
    for (auto const& m : epochs[0].measurements) {
        if (std::string(m.satellite_id.name()) != "G04") continue;
        if (m.signal_id == SignalId::GPS_L1_CA) has_l1ca = true;
        if (m.signal_id == SignalId::GPS_L5_Q) has_l5q = true;
    }
    CHECK(has_l1ca);
    CHECK(has_l5q);
}

TEST_CASE("RINEX obs - multiple epochs") {
    char const* content =
        R"(     3.04           OBSERVATION DATA    G (GPS)             RINEX VERSION / TYPE
G    1 C1C                                                  SYS / # / OBS TYPES
                                                            END OF HEADER
> 2026 03 17  0  0  0.0000000  0  1
G04  24939143.384
> 2026 03 17  0  0 30.0000000  0  1
G04  24939200.000
)";
    auto path = write_tmp(content);

    std::vector<format::rinex::ObsEpoch> epochs;
    format::rinex::parse_obs(path, [&](format::rinex::ObsEpoch const& e) {
        epochs.push_back(e);
    });
    unlink(path.c_str());

    CHECK(epochs.size() == 2);
    double pr0 = 0.0, pr1 = 0.0;
    for (auto const& m : epochs[0].measurements)
        if (m.signal_id == SignalId::GPS_L1_CA) pr0 = m.pseudorange;
    for (auto const& m : epochs[1].measurements)
        if (m.signal_id == SignalId::GPS_L1_CA) pr1 = m.pseudorange;
    CHECK(pr0 == doctest::Approx(24939143.384));
    CHECK(pr1 == doctest::Approx(24939200.000));
}
