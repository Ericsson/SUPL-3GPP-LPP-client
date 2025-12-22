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

TEST_CASE("Timestamp as_double") {
    ts::Timestamp t(100, 0.5);
    CHECK(t.as_double() == doctest::Approx(100.5));
    CHECK(t.as_double() == t.full_seconds());
}

TEST_CASE("Timestamp scalar addition") {
    ts::Timestamp t(100, 0.3);
    auto          result = t + 10.5;
    CHECK(result.seconds() == 110);
    CHECK(result.fraction() == doctest::Approx(0.8));
}

TEST_CASE("Timestamp scalar subtraction") {
    ts::Timestamp t(100, 0.3);
    auto          result = t - 10.1;
    CHECK(result.seconds() == 90);
    CHECK(result.fraction() == doctest::Approx(0.2));
}

TEST_CASE("Timestamp += operator") {
    ts::Timestamp t(100, 0.3);
    t += 10.5;
    CHECK(t.seconds() == 110);
    CHECK(t.fraction() == doctest::Approx(0.8));
}

TEST_CASE("Timestamp -= operator") {
    ts::Timestamp t(100, 0.3);
    t -= 10.1;
    CHECK(t.seconds() == 90);
    CHECK(t.fraction() == doctest::Approx(0.2));
}

TEST_CASE("Timestamp != operator") {
    ts::Timestamp t1(100, 0.5);
    ts::Timestamp t2(100, 0.6);
    ts::Timestamp t3(100, 0.5);
    CHECK(t1 != t2);
    CHECK_FALSE(t1 != t3);
}

TEST_CASE("Timestamp comparison precision") {
    ts::Timestamp t1(100, 0.0);
    ts::Timestamp t2(100, 1e-8);
    CHECK(t2 > t1);
    CHECK(t1 < t2);
    CHECK_FALSE(t1 == t2);
}

TEST_CASE("Double loses precision at large values - Timestamp preserves it") {
    // At year 2100 from GPS epoch: ~3.8 billion seconds
    // double only has ~15 significant digits, so at 3.8e9 seconds,
    // precision is limited to ~420 nanoseconds (3.8e9 * 2^-52 ≈ 4.2e-7)
    constexpr int64_t LARGE_TIME = 3800000000LL;  // ~120 years in seconds
    constexpr double  SMALL_DIFF = 1e-7;          // 100 nanoseconds

    // Using double: loses precision
    double d1 = static_cast<double>(LARGE_TIME);
    double d2 = static_cast<double>(LARGE_TIME) + SMALL_DIFF;
    CHECK(d1 == d2);  // FAILS to distinguish - precision lost!

    // Using Timestamp: preserves precision
    ts::Timestamp ts1(LARGE_TIME, 0.0);
    ts::Timestamp ts2(LARGE_TIME, SMALL_DIFF);
    CHECK(ts2 > ts1);         // Correctly distinguishes
    CHECK_FALSE(ts1 == ts2);  // Correctly not equal
}

TEST_CASE("Double arithmetic loses sub-microsecond precision at GPS week 2100") {
    // GPS week 2100 ≈ 1.27 billion seconds from GPS epoch
    constexpr int64_t GPS_WEEK_2100_SECONDS = 2100LL * ts::WEEK_IN_SECONDS;
    constexpr double  NANOSECONDS_100       = 1e-7;

    // Double arithmetic: adding small value gets lost
    double base   = static_cast<double>(GPS_WEEK_2100_SECONDS);
    double result = base + NANOSECONDS_100;
    CHECK(result - base == 0.0);  // Lost the 100ns!

    // Timestamp arithmetic: preserves the small value
    ts::Timestamp ts_base(GPS_WEEK_2100_SECONDS, 0.0);
    ts::Timestamp ts_result = ts_base + NANOSECONDS_100;
    CHECK(ts_result.fraction() == doctest::Approx(NANOSECONDS_100));  // Preserved!
}
