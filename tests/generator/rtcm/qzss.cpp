#include <doctest/doctest.h>
#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>

TEST_CASE("QZSS SatelliteId PRN range") {
    SUBCASE("Valid PRN range 193-202") {
        for (uint8_t prn = 193; prn <= 202; prn++) {
            auto sv_id = SatelliteId::from_qzs_prn(prn);
            CHECK(sv_id.is_valid());
            CHECK(sv_id.is_qzss());
            CHECK(sv_id.gnss() == SatelliteId::Gnss::QZSS);
            CHECK(sv_id.prn().value == (prn - 193 + 1));
        }
    }

    SUBCASE("Invalid PRN below range") {
        auto sv_id = SatelliteId::from_qzs_prn(192);
        CHECK_FALSE(sv_id.is_valid());
    }

    SUBCASE("Invalid PRN above range") {
        auto sv_id = SatelliteId::from_qzs_prn(203);
        CHECK_FALSE(sv_id.is_valid());
    }

    SUBCASE("Known satellites") {
        auto qzs1 = SatelliteId::from_qzs_prn(193);
        CHECK(qzs1.is_valid());
        CHECK(qzs1.name() == std::string("J01"));

        auto qzs2 = SatelliteId::from_qzs_prn(194);
        CHECK(qzs2.is_valid());
        CHECK(qzs2.name() == std::string("J02"));

        auto qzs3 = SatelliteId::from_qzs_prn(199);
        CHECK(qzs3.is_valid());
        CHECK(qzs3.name() == std::string("J07"));
    }
}

TEST_CASE("QZSS SatelliteId string conversion") {
    SUBCASE("to_string") {
        auto sv_id = SatelliteId::from_qzs_prn(193);
        CHECK(sv_id.to_string() == "J00");
    }

    SUBCASE("from_string") {
        auto sv_id = SatelliteId::from_string("J01");
        CHECK(sv_id.is_valid());
        CHECK(sv_id.is_qzss());
    }
}

TEST_CASE("QZSS SignalId frequencies") {
    SUBCASE("L1 C/A frequency") {
        CHECK(SignalId::QZSS_L1_CA.frequency() == doctest::Approx(1575.42e3));
        CHECK(SignalId::QZSS_L1_CA.frequency_type() == FrequencyType::L1);
    }

    SUBCASE("L1C frequencies") {
        CHECK(SignalId::QZSS_L1C_D.frequency() == doctest::Approx(1575.42e3));
        CHECK(SignalId::QZSS_L1C_P.frequency() == doctest::Approx(1575.42e3));
        CHECK(SignalId::QZSS_L1C_D_P.frequency() == doctest::Approx(1575.42e3));
    }

    SUBCASE("L2C frequencies") {
        CHECK(SignalId::QZSS_L2C_M.frequency() == doctest::Approx(1227.60e3));
        CHECK(SignalId::QZSS_L2C_L.frequency() == doctest::Approx(1227.60e3));
        CHECK(SignalId::QZSS_L2C_M_L.frequency() == doctest::Approx(1227.60e3));
        CHECK(SignalId::QZSS_L2C_M.frequency_type() == FrequencyType::L2);
    }

    SUBCASE("L5 frequencies") {
        CHECK(SignalId::QZSS_L5_I.frequency() == doctest::Approx(1176.45e3));
        CHECK(SignalId::QZSS_L5_Q.frequency() == doctest::Approx(1176.45e3));
        CHECK(SignalId::QZSS_L5_I_Q.frequency() == doctest::Approx(1176.45e3));
        CHECK(SignalId::QZSS_L5_I.frequency_type() == FrequencyType::L5);
    }

    SUBCASE("L6 frequencies (QZSS-specific)") {
        CHECK(SignalId::QZSS_L6_D.frequency() == doctest::Approx(1278.75e3));
        CHECK(SignalId::QZSS_L6_E.frequency() == doctest::Approx(1278.75e3));
        CHECK(SignalId::QZSS_L6_D.frequency_type() == FrequencyType::L6);
    }
}

TEST_CASE("QZSS SignalId wavelengths") {
    SUBCASE("L1 wavelength") {
        double wavelength = SignalId::QZSS_L1_CA.wavelength();
        CHECK(wavelength == doctest::Approx(190.29).epsilon(0.01));
    }

    SUBCASE("L5 wavelength") {
        double wavelength = SignalId::QZSS_L5_I.wavelength();
        CHECK(wavelength == doctest::Approx(254.83).epsilon(0.01));
    }
}
