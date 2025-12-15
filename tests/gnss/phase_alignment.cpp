#include <doctest/doctest.h>
#include <gnss/signal_id.hpp>

TEST_CASE("GPS phase alignment") {
    CHECK(SignalId::GPS_L1_CA.phase_alignment_shift() == 0.0);
    CHECK(SignalId::GPS_L2_C_A.phase_alignment_shift() == 0.0);
    CHECK(SignalId::GPS_L2_L2C_L.phase_alignment_shift() == 0.25);
}

TEST_CASE("GLONASS phase alignment") {
    CHECK(SignalId::GLONASS_G1_P.phase_alignment_shift() == 0.0);
    CHECK(SignalId::GLONASS_G1_CA.phase_alignment_shift() == 0.0);

    CHECK(SignalId::GLONASS_G2_P.phase_alignment_shift() == 0.0);
    CHECK(SignalId::GLONASS_G2_CA.phase_alignment_shift() == 0.0);
}

TEST_CASE("Galileo phase alignment") {
    CHECK(SignalId::GALILEO_E1_A.phase_alignment_shift() == 0.0);
    CHECK(SignalId::GALILEO_E1_B_I_NAV_OS_CS_SOL.phase_alignment_shift() == 0.0);

    CHECK(SignalId::GALILEO_E5A_Q.phase_alignment_shift() == 0.0);
    CHECK(SignalId::GALILEO_E5A_I.phase_alignment_shift() == 0.0);

    CHECK(SignalId::GALILEO_E5B_Q.phase_alignment_shift() == 0.0);
    CHECK(SignalId::GALILEO_E6_B.phase_alignment_shift() == 0.0);
}

TEST_CASE("QZSS phase alignment") {
    CHECK(SignalId::QZSS_L1_CA.phase_alignment_shift() == 0.0);
    CHECK(SignalId::QZSS_L1C_D.phase_alignment_shift() == 0.0);

    CHECK(SignalId::QZSS_L2C_L.phase_alignment_shift() == 0.25);
    CHECK(SignalId::QZSS_L5_Q.phase_alignment_shift() == 0.0);
}

TEST_CASE("BeiDou phase alignment") {
    CHECK(SignalId::BEIDOU_B1_Q.phase_alignment_shift() == 0.0);
    CHECK(SignalId::BEIDOU_B1_I.phase_alignment_shift() == 0.0);

    CHECK(SignalId::BEIDOU_B1C_P.phase_alignment_shift() == 0.0);
    CHECK(SignalId::BEIDOU_B2_Q.phase_alignment_shift() == 0.0);
    CHECK(SignalId::BEIDOU_B3_Q.phase_alignment_shift() == 0.0);
    CHECK(SignalId::BEIDOU_B2A_P.phase_alignment_shift() == 0.0);
}

TEST_CASE("Cross-frequency returns zero") {
    CHECK(SignalId::GPS_L1_CA.phase_alignment_shift() == 0.0);
    CHECK(SignalId::GPS_L5_I.phase_alignment_shift() == 0.0);
}

TEST_CASE("Cross-GNSS returns zero") {
    CHECK(SignalId::GPS_L1_CA.phase_alignment_shift() == 0.0);
    CHECK(SignalId::GLONASS_G1_CA.phase_alignment_shift() == 0.0);
}
