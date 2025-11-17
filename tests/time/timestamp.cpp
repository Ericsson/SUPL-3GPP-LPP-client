#include <doctest/doctest.h>
#include <time/timestamp.hpp>

TEST_CASE("Timestamp basic operations") {
    ts::Timestamp t1(100, 0.5);
    CHECK(t1.seconds() == 100);
    CHECK(t1.fraction() == doctest::Approx(0.5));
    CHECK(t1.full_seconds() == doctest::Approx(100.5));
}

TEST_CASE("Timestamp addition") {
    ts::Timestamp t1(100, 0.3);
    ts::Timestamp t2(50, 0.8);
    auto          result = t1 + t2;
    CHECK(result.seconds() == 151);
    CHECK(result.fraction() == doctest::Approx(0.1));
}

TEST_CASE("Timestamp subtraction") {
    ts::Timestamp t1(100, 0.3);
    ts::Timestamp t2(50, 0.1);
    auto          result = t1 - t2;
    CHECK(result.seconds() == 50);
    CHECK(result.fraction() == doctest::Approx(0.2));
}

TEST_CASE("Timestamp negative fraction normalization") {
    ts::Timestamp t(100, -0.5);
    CHECK(t.seconds() == 99);
    CHECK(t.fraction() == doctest::Approx(0.5));
}

TEST_CASE("Timestamp large fraction normalization") {
    ts::Timestamp t(100, 2.7);
    CHECK(t.seconds() == 102);
    CHECK(t.fraction() == doctest::Approx(0.7));
}
