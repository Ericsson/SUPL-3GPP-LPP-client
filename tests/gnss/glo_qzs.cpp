#include <doctest/doctest.h>
#include <gnss/satellite_id.hpp>
#include <gnss/signal_id.hpp>

#include <cstring>

TEST_CASE("QZSS signal ids") {
    // RTCM 10403.4 Table 3.5-111
    CHECK(SignalId::QZSS_L1_CA.as_msm().value == 2);
    CHECK(SignalId::QZSS_L6_D.as_msm().value == 9);
    CHECK(SignalId::QZSS_L2C_M.as_msm().value == 15);
    CHECK(SignalId::QZSS_L2C_M_L.as_msm().value == 17);
    CHECK(SignalId::QZSS_L5_I.as_msm().value == 22);
    CHECK(SignalId::QZSS_L5_I_Q.as_msm().value == 24);
    CHECK(SignalId::QZSS_L1C_D.as_msm().value == 30);
    CHECK(SignalId::QZSS_L1C_D_P.as_msm().value == 32);
    CHECK_FALSE(SignalId::QZSS_L6_E.as_msm().valid);

    CHECK(SignalId::QZSS_L1_CA.to_rinex() == "1C");
    CHECK(SignalId::QZSS_L5_I_Q.to_rinex() == "5X");
    CHECK(std::strcmp(SignalId::QZSS_L2C_L.name(), "L2C(L)") == 0);
}

TEST_CASE("QZSS satellite ids") {
    auto j01 = SatelliteId::from_qzs_prn(193);
    REQUIRE(j01.is_valid());
    CHECK(j01.as_msm().value == 1);
    CHECK(std::strcmp(j01.name(), "J01") == 0);
}

TEST_CASE("GLONASS FDMA frequencies") {
    // ICD 5.0: G1 = 1602 + k * 0.5625 MHz, G2 = 1246 + k * 0.4375 MHz
    CHECK(SignalId::GLONASS_G1_CA.frequency(0) == doctest::Approx(1602000.0));
    CHECK(SignalId::GLONASS_G1_CA.frequency(-7) == doctest::Approx(1598062.5));
    CHECK(SignalId::GLONASS_G1_P.frequency(6) == doctest::Approx(1605375.0));
    CHECK(SignalId::GLONASS_G2_CA.frequency(1) == doctest::Approx(1246437.5));
    CHECK(SignalId::GLONASS_G2_P.frequency(-7) == doctest::Approx(1242937.5));
    // CDMA signals have no channel offset
    CHECK(SignalId::GLONASS_G3_I.frequency(5) ==
          doctest::Approx(SignalId::GLONASS_G3_I.frequency()));
    // other GNSS ignore the channel
    CHECK(SignalId::GPS_L1_CA.frequency(3) == doctest::Approx(SignalId::GPS_L1_CA.frequency()));

    auto r05 = SatelliteId::from_glo_slot(5);
    REQUIRE(r05.is_valid());
    CHECK(r05.as_msm().value == 5);
    CHECK(std::strcmp(r05.name(), "R05") == 0);
    CHECK_FALSE(SatelliteId::from_glo_slot(25).is_valid());
}
