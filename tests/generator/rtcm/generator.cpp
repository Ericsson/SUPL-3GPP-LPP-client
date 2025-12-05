#include <doctest/doctest.h>
#include "helpers.hpp"

TEST_CASE("RTCM Generator - Default Filter") {
    auto test_cases = find_test_cases("tests/data/rtcm/captured/default-*.uper");

    if (test_cases.empty()) {
        WARN("No test data found for default filter");
        return;
    }

    for (auto const& test_case : test_cases) {
        CAPTURE(test_case.name);

        auto lpp = load_lpp_uper(test_case.uper_path);
        REQUIRE(lpp != nullptr);

        generator::rtcm::Generator     gen;
        generator::rtcm::MessageFilter filter;

        auto messages = gen.generate(lpp, filter);
        auto golden   = load_rtcm_messages(test_case.rtcm_path);

        REQUIRE(messages.size() == golden.size());
        for (size_t i = 0; i < messages.size(); i++) {
            CAPTURE(i);
            CAPTURE(messages[i].id());
            CHECK(messages[i].data() == golden[i]);
        }

        ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp);
    }
}

TEST_CASE("RTCM Generator - GPS Only") {
    auto test_cases = find_test_cases("tests/data/rtcm/captured/gps_only-*.uper");

    if (test_cases.empty()) {
        WARN("No test data found for GPS only");
        return;
    }

    for (auto const& test_case : test_cases) {
        CAPTURE(test_case.name);

        auto lpp = load_lpp_uper(test_case.uper_path);
        REQUIRE(lpp != nullptr);

        generator::rtcm::Generator     gen;
        generator::rtcm::MessageFilter filter;
        filter.systems.glonass = false;
        filter.systems.galileo = false;
        filter.systems.beidou  = false;

        auto messages = gen.generate(lpp, filter);
        auto golden   = load_rtcm_messages(test_case.rtcm_path);

        REQUIRE(messages.size() == golden.size());
        for (size_t i = 0; i < messages.size(); i++) {
            CAPTURE(i);
            CAPTURE(messages[i].id());
            CHECK(messages[i].data() == golden[i]);
        }

        ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp);
    }
}

TEST_CASE("RTCM Generator - MSM4 Forced") {
    auto test_cases = find_test_cases("tests/data/rtcm/captured/msm4-*.uper");

    if (test_cases.empty()) {
        WARN("No test data found for MSM4");
        return;
    }

    for (auto const& test_case : test_cases) {
        CAPTURE(test_case.name);

        auto lpp = load_lpp_uper(test_case.uper_path);
        REQUIRE(lpp != nullptr);

        generator::rtcm::Generator     gen;
        generator::rtcm::MessageFilter filter;
        filter.msm.force_msm4 = true;

        auto messages = gen.generate(lpp, filter);
        auto golden   = load_rtcm_messages(test_case.rtcm_path);

        REQUIRE(messages.size() == golden.size());
        for (size_t i = 0; i < messages.size(); i++) {
            CAPTURE(i);
            CAPTURE(messages[i].id());
            CHECK(messages[i].data() == golden[i]);
        }

        ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp);
    }
}

TEST_CASE("RTCM Generator - MSM7 Forced") {
    auto test_cases = find_test_cases("tests/data/rtcm/captured/msm7-*.uper");

    if (test_cases.empty()) {
        WARN("No test data found for MSM7");
        return;
    }

    for (auto const& test_case : test_cases) {
        CAPTURE(test_case.name);

        auto lpp = load_lpp_uper(test_case.uper_path);
        REQUIRE(lpp != nullptr);

        generator::rtcm::Generator     gen;
        generator::rtcm::MessageFilter filter;
        filter.msm.force_msm7 = true;

        auto messages = gen.generate(lpp, filter);
        auto golden   = load_rtcm_messages(test_case.rtcm_path);

        REQUIRE(messages.size() == golden.size());
        for (size_t i = 0; i < messages.size(); i++) {
            CAPTURE(i);
            CAPTURE(messages[i].id());
            CHECK(messages[i].data() == golden[i]);
        }

        ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp);
    }
}

TEST_CASE("RTCM Generator - Multi-GNSS") {
    auto test_cases = find_test_cases("tests/data/rtcm/captured/multi_gnss-*.uper");

    if (test_cases.empty()) {
        WARN("No test data found for multi-GNSS");
        return;
    }

    for (auto const& test_case : test_cases) {
        CAPTURE(test_case.name);

        auto lpp = load_lpp_uper(test_case.uper_path);
        REQUIRE(lpp != nullptr);

        generator::rtcm::Generator     gen;
        generator::rtcm::MessageFilter filter;

        auto messages = gen.generate(lpp, filter);
        auto golden   = load_rtcm_messages(test_case.rtcm_path);

        REQUIRE(messages.size() == golden.size());
        for (size_t i = 0; i < messages.size(); i++) {
            CAPTURE(i);
            CAPTURE(messages[i].id());
            CHECK(messages[i].data() == golden[i]);
        }

        ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp);
    }
}

TEST_CASE("RTCM Generator - With Residuals") {
    auto test_cases = find_test_cases("tests/data/rtcm/captured/with_residuals-*.uper");

    if (test_cases.empty()) {
        WARN("No test data found for residuals");
        return;
    }

    for (auto const& test_case : test_cases) {
        CAPTURE(test_case.name);

        auto lpp = load_lpp_uper(test_case.uper_path);
        REQUIRE(lpp != nullptr);

        generator::rtcm::Generator     gen;
        generator::rtcm::MessageFilter filter;
        filter.residuals.mt1030 = true;
        filter.residuals.mt1031 = true;

        auto messages = gen.generate(lpp, filter);
        auto golden   = load_rtcm_messages(test_case.rtcm_path);

        REQUIRE(messages.size() == golden.size());
        for (size_t i = 0; i < messages.size(); i++) {
            CAPTURE(i);
            CAPTURE(messages[i].id());
            CHECK(messages[i].data() == golden[i]);
        }

        ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp);
    }
}

TEST_CASE("RTCM Generator - With Bias Information") {
    auto test_cases = find_test_cases("tests/data/rtcm/captured/with_bias-*.uper");

    if (test_cases.empty()) {
        WARN("No test data found for bias information");
        return;
    }

    for (auto const& test_case : test_cases) {
        CAPTURE(test_case.name);

        auto lpp = load_lpp_uper(test_case.uper_path);
        REQUIRE(lpp != nullptr);

        generator::rtcm::Generator     gen;
        generator::rtcm::MessageFilter filter;
        filter.bias.mt1230 = true;

        auto messages = gen.generate(lpp, filter);
        auto golden   = load_rtcm_messages(test_case.rtcm_path);

        REQUIRE(messages.size() == golden.size());
        for (size_t i = 0; i < messages.size(); i++) {
            CAPTURE(i);
            CAPTURE(messages[i].id());
            CHECK(messages[i].data() == golden[i]);
        }

        ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp);
    }
}
