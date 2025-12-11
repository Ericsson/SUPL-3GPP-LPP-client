#include <cmath>
#include <doctest/doctest.h>
#include <lpp/location_information.hpp>

constexpr double TOLERANCE      = 0.01;
constexpr double CONF_TOLERANCE = 0.0001;

TEST_SUITE("HorizontalAccuracy") {
    TEST_CASE("Basic 1-sigma (39% confidence)") {
        auto ha = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);

        CHECK(ha.semi_major == doctest::Approx(10.0).epsilon(TOLERANCE));
        CHECK(ha.semi_minor == doctest::Approx(10.0).epsilon(TOLERANCE));
        CHECK(ha.confidence == doctest::Approx(0.393469).epsilon(CONF_TOLERANCE));
        CHECK(ha.orientation == doctest::Approx(0.0).epsilon(TOLERANCE));
    }

    TEST_CASE("68% confidence") {
        auto ha    = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_68 = ha.to_68();

        CHECK(ha_68.semi_major == doctest::Approx(15.152).epsilon(TOLERANCE));
        CHECK(ha_68.confidence == doctest::Approx(0.6827).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("95% confidence") {
        auto ha    = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_95 = ha.to_95();

        CHECK(ha_95.semi_major == doctest::Approx(24.477).epsilon(TOLERANCE));
        CHECK(ha_95.confidence == doctest::Approx(0.95).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("Tiny ellipse") {
        auto ha = lpp::HorizontalAccuracy::from_1sigma(0.1, 0.1, 0);

        CHECK(ha.semi_major == doctest::Approx(0.1).epsilon(TOLERANCE));
        CHECK(ha.confidence == doctest::Approx(0.393469).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("Large ellipse") {
        auto ha    = lpp::HorizontalAccuracy::from_1sigma(1000.0, 1000.0, 0);
        auto ha_68 = ha.to_68();

        CHECK(ha_68.semi_major == doctest::Approx(1515.195).epsilon(TOLERANCE));
        CHECK(ha_68.confidence == doctest::Approx(0.6827).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("Low confidence (10%)") {
        auto ha    = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_10 = ha.to_confidence(0.10);

        CHECK(ha_10.confidence == doctest::Approx(0.10).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("High confidence (99%)") {
        auto ha    = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_99 = ha.to_confidence(0.99);

        CHECK(ha_99.confidence == doctest::Approx(0.99).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("Convert 39% to 68%") {
        auto ha     = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_in  = ha.to_confidence(0.393469340287367);
        auto ha_out = ha_in.to_68();

        CHECK(ha_out.semi_major == doctest::Approx(15.151948).epsilon(TOLERANCE));
        CHECK(ha_out.confidence == doctest::Approx(0.682689492137086).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("Convert 39% to 95%") {
        auto ha     = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_in  = ha.to_confidence(0.393469340287367);
        auto ha_out = ha_in.to_95();

        CHECK(ha_out.semi_major == doctest::Approx(24.477468).epsilon(TOLERANCE));
        CHECK(ha_out.confidence == doctest::Approx(0.95).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("Convert 68% to 95%") {
        auto ha     = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_in  = ha.to_confidence(0.682689492137086);
        auto ha_out = ha_in.to_95();

        CHECK(ha_out.semi_major == doctest::Approx(24.477553).epsilon(TOLERANCE));
        CHECK(ha_out.confidence == doctest::Approx(0.95).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("Convert 95% to 68%") {
        auto ha     = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_in  = ha.to_confidence(0.95);
        auto ha_out = ha_in.to_68();

        CHECK(ha_out.semi_major == doctest::Approx(15.151658).epsilon(TOLERANCE));
        CHECK(ha_out.confidence == doctest::Approx(0.682689492137086).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("Convert 95% to 39%") {
        auto ha     = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_in  = ha.to_confidence(0.95);
        auto ha_out = ha_in.to_39();

        CHECK(ha_out.semi_major == doctest::Approx(9.999809).epsilon(TOLERANCE));
        CHECK(ha_out.confidence == doctest::Approx(0.393469340287367).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("Convert 68% to 39%") {
        auto ha     = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_in  = ha.to_confidence(0.682689492137086);
        auto ha_out = ha_in.to_39();

        CHECK(ha_out.semi_major == doctest::Approx(10.000035).epsilon(TOLERANCE));
        CHECK(ha_out.confidence == doctest::Approx(0.393469340287367).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("Round trip 39% -> 95% -> 39%") {
        auto ha      = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_95   = ha.to_95();
        auto ha_back = ha_95.to_39();

        CHECK(ha_back.semi_major == doctest::Approx(10.0).epsilon(TOLERANCE));
        CHECK(ha_back.confidence == doctest::Approx(0.393469).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("from_h_acc_95_rss with 20m") {
        auto ha = lpp::HorizontalAccuracy::from_h_acc_95_rss(20.0, 0);

        CHECK(ha.semi_major == doctest::Approx(5.777614).epsilon(TOLERANCE));
        CHECK(ha.semi_minor == doctest::Approx(5.777614).epsilon(TOLERANCE));
        CHECK(ha.confidence == doctest::Approx(0.393469).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("from_h_acc_95_radius with 20m") {
        auto ha = lpp::HorizontalAccuracy::from_h_acc_95_radius(20.0, 0);

        CHECK(ha.semi_major == doctest::Approx(8.170780).epsilon(TOLERANCE));
        CHECK(ha.semi_minor == doctest::Approx(8.170780).epsilon(TOLERANCE));
        CHECK(ha.confidence == doctest::Approx(0.393469).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("from_h_acc_95_rss with 5m") {
        auto ha = lpp::HorizontalAccuracy::from_h_acc_95_rss(5.0, 0);

        CHECK(ha.semi_major == doctest::Approx(1.444403).epsilon(TOLERANCE));
        CHECK(ha.semi_minor == doctest::Approx(1.444403).epsilon(TOLERANCE));
        CHECK(ha.confidence == doctest::Approx(0.393469).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("from_h_acc_95_radius with 100m") {
        auto ha = lpp::HorizontalAccuracy::from_h_acc_95_radius(100.0, 0);

        CHECK(ha.semi_major == doctest::Approx(40.853898).epsilon(TOLERANCE));
        CHECK(ha.semi_minor == doctest::Approx(40.853898).epsilon(TOLERANCE));
        CHECK(ha.confidence == doctest::Approx(0.393469).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("Micro sigma") {
        auto ha = lpp::HorizontalAccuracy::from_1sigma(0.001, 0.001, 0);

        CHECK(ha.semi_major == doctest::Approx(0.001).epsilon(TOLERANCE));
        CHECK(ha.confidence == doctest::Approx(0.393469).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("Odd confidence 73.5%") {
        auto ha    = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_73 = ha.to_confidence(0.735);

        CHECK(ha_73.confidence == doctest::Approx(0.735).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("50% confidence (CEP)") {
        auto ha    = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_50 = ha.to_confidence(0.50);

        CHECK(ha_50.confidence == doctest::Approx(0.50).epsilon(CONF_TOLERANCE));
        CHECK(ha_50.semi_major == doctest::Approx(11.774).epsilon(TOLERANCE));
    }

    TEST_CASE("Chain conversion 39->68->95->68->39") {
        auto ha         = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_68      = ha.to_68();
        auto ha_95      = ha_68.to_95();
        auto ha_68_back = ha_95.to_68();
        auto ha_39_back = ha_68_back.to_39();

        CHECK(ha_39_back.semi_major == doctest::Approx(10.0).epsilon(TOLERANCE));
        CHECK(ha_39_back.confidence == doctest::Approx(0.393469).epsilon(CONF_TOLERANCE));
    }

    TEST_CASE("chi2_ppf formula verification") {
        SUBCASE("10% confidence") {
            double chi2 = -2.0 * std::log(1.0 - 0.10);
            CHECK(chi2 == doctest::Approx(0.210721).epsilon(0.00001));
        }

        SUBCASE("50% confidence") {
            double chi2 = -2.0 * std::log(1.0 - 0.50);
            CHECK(chi2 == doctest::Approx(1.386294).epsilon(0.00001));
        }

        SUBCASE("95% confidence") {
            double chi2 = -2.0 * std::log(1.0 - 0.95);
            CHECK(chi2 == doctest::Approx(5.991465).epsilon(0.00001));
        }

        SUBCASE("99% confidence") {
            double chi2 = -2.0 * std::log(1.0 - 0.99);
            CHECK(chi2 == doctest::Approx(9.210340).epsilon(0.00001));
        }
    }

    TEST_CASE("Practical UBX scenario") {
        auto ha = lpp::HorizontalAccuracy::from_h_acc_95_rss(15.0, 0);

        SUBCASE("Initial 39% confidence") {
            CHECK(ha.semi_major == doctest::Approx(4.3332).epsilon(TOLERANCE));
            CHECK(ha.confidence == doctest::Approx(0.393469).epsilon(CONF_TOLERANCE));
        }

        SUBCASE("Convert to 68%") {
            auto ha_68 = ha.to_68();
            CHECK(ha_68.semi_major == doctest::Approx(6.5657).epsilon(TOLERANCE));
            CHECK(ha_68.confidence == doctest::Approx(0.6827).epsilon(CONF_TOLERANCE));
        }

        SUBCASE("Convert to 95%") {
            auto ha_95 = ha.to_95();
            CHECK(ha_95.semi_major == doctest::Approx(10.6066).epsilon(TOLERANCE));
            CHECK(ha_95.confidence == doctest::Approx(0.95).epsilon(CONF_TOLERANCE));
        }
    }

    TEST_CASE("Identity conversions") {
        auto ha_39 = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
        auto ha_68 = ha_39.to_68();
        auto ha_95 = ha_39.to_95();

        SUBCASE("39% to 39%") {
            auto ha_same = ha_39.to_39();
            CHECK(ha_same.semi_major == doctest::Approx(ha_39.semi_major).epsilon(0.0001));
            CHECK(ha_same.confidence == doctest::Approx(ha_39.confidence).epsilon(CONF_TOLERANCE));
        }

        SUBCASE("68% to 68%") {
            auto ha_same = ha_68.to_68();
            CHECK(ha_same.semi_major == doctest::Approx(ha_68.semi_major).epsilon(0.0001));
            CHECK(ha_same.confidence == doctest::Approx(ha_68.confidence).epsilon(CONF_TOLERANCE));
        }

        SUBCASE("95% to 95%") {
            auto ha_same = ha_95.to_95();
            CHECK(ha_same.semi_major == doctest::Approx(ha_95.semi_major).epsilon(0.0001));
            CHECK(ha_same.confidence == doctest::Approx(ha_95.confidence).epsilon(CONF_TOLERANCE));
        }
    }

    TEST_CASE("Orientation normalization") {
        SUBCASE("Negative orientation") {
            auto ha = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, -45.0);
            CHECK(ha.orientation == doctest::Approx(135.0).epsilon(TOLERANCE));
        }

        SUBCASE("Orientation >= 180") {
            auto ha = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 270.0);
            CHECK(ha.orientation == doctest::Approx(90.0).epsilon(TOLERANCE));
        }

        SUBCASE("Multiple wraps") {
            auto ha = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, -540.0);
            CHECK(ha.orientation == doctest::Approx(0.0).epsilon(TOLERANCE));
        }
    }

    TEST_CASE("Ellipse vs Circle") {
        SUBCASE("Circular (semi_major = semi_minor)") {
            auto ha = lpp::HorizontalAccuracy::from_1sigma(10.0, 10.0, 0);
            CHECK(ha.semi_major == ha.semi_minor);
        }

        SUBCASE("Elliptical (semi_major != semi_minor)") {
            auto ha = lpp::HorizontalAccuracy::from_1sigma(15.0, 10.0, 45.0);
            CHECK(ha.semi_major == doctest::Approx(15.0).epsilon(TOLERANCE));
            CHECK(ha.semi_minor == doctest::Approx(10.0).epsilon(TOLERANCE));
            CHECK(ha.orientation == doctest::Approx(45.0).epsilon(TOLERANCE));
        }
    }

    TEST_CASE("Extreme values") {
        SUBCASE("Very small h_acc") {
            auto ha = lpp::HorizontalAccuracy::from_h_acc_95_rss(0.01, 0);
            CHECK(ha.semi_major > 0.0);
            CHECK(ha.confidence == doctest::Approx(0.393469).epsilon(CONF_TOLERANCE));
        }

        SUBCASE("Very large h_acc") {
            auto ha = lpp::HorizontalAccuracy::from_h_acc_95_rss(10000.0, 0);
            CHECK(ha.semi_major == doctest::Approx(2888.854).epsilon(TOLERANCE));
            CHECK(ha.confidence == doctest::Approx(0.393469).epsilon(CONF_TOLERANCE));
        }
    }

    TEST_CASE("Comparison: RSS vs Radius") {
        double h_acc     = 20.0;
        auto   ha_rss    = lpp::HorizontalAccuracy::from_h_acc_95_rss(h_acc, 0);
        auto   ha_radius = lpp::HorizontalAccuracy::from_h_acc_95_radius(h_acc, 0);

        CHECK(ha_rss.semi_major < ha_radius.semi_major);

        double ratio = ha_radius.semi_major / ha_rss.semi_major;
        CHECK(ratio == doctest::Approx(std::sqrt(2.0)).epsilon(0.01));
    }
}
