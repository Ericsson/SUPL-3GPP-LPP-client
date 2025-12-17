#include <doctest/doctest.h>
#include "helpers.hpp"

TEST_CASE("RTCM Generator - Default Filter") {
    auto test_cases = find_test_cases("tests/data/rtcm/captured/default-*.uper");
    REQUIRE(test_cases.size() > 0);

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
    REQUIRE(test_cases.size() > 0);

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
    REQUIRE(test_cases.size() > 0);

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
    REQUIRE(test_cases.size() > 0);

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
