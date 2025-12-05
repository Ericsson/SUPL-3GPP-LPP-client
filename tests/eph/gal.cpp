#include <cstring>
#include <doctest/doctest.h>
#include <ephemeris/gal.hpp>
#include <fstream>
#include <msgpack/msgpack.hpp>
#include <msgpack/vector.hpp>
#include <test_utils.hpp>
#include <time/gps.hpp>
#include <time/utc.hpp>
#include <vector>

struct Test {
    int64_t     gps_sec;
    int64_t     offset;
    std::string time_str;
    double      x, y, z;
    double      x2, y2, z2;
    double      vx, vy, vz;
    double      clock_bias, clock_drift;

    MSGPACK_DEFINE(gps_sec, offset, time_str, x, y, z, x2, y2, z2, vx, vy, vz, clock_bias,
                   clock_drift)
};

static std::vector<std::string> find_gal_files() {
    char const* paths[] = {"../../tests/data/gal", "../tests/data/gal"};
    return test_utils::find_files_with_suffix(paths, 2, ".msgpack");
}

TEST_CASE("Galileo ephemeris computation") {
    auto files = find_gal_files();
    REQUIRE(!files.empty());

    for (auto const& filename : files) {
        std::ifstream f(filename, std::ios::binary);
        REQUIRE(f.is_open());
        CAPTURE(filename);

        std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(f)),
                                    std::istreambuf_iterator<char>());
        msgpack::Unpacker    unpacker(buffer.data(), buffer.size());

        uint32_t num_ephemerides = 0;
        REQUIRE(unpacker.unpack_array_header(num_ephemerides));

        int count = 0;
        for (uint32_t i = 0; i < num_ephemerides; i++) {
            uint32_t eph_size = 0;
            REQUIRE(unpacker.unpack_array_header(eph_size));
            REQUIRE(eph_size == 2);

            ephemeris::GalEphemeris eph;
            REQUIRE(msgpack::unpack(unpacker, eph));

            std::vector<Test> tests;
            REQUIRE(msgpack::unpack(unpacker, tests));

            for (auto const& test : tests) {
                auto gps_time = ts::Gps{ts::Timestamp{test.gps_sec}};
                auto time     = ts::Gst{gps_time};

                CAPTURE(eph.prn);
                CAPTURE(eph.toe);
                CAPTURE(eph.toc);
                CAPTURE(eph.iod_nav);
                CAPTURE(eph.week_number);
                CAPTURE(test.offset);

                CHECK(ts::Utc{gps_time}.rtklib_time_string(3) == test.time_str);
                CHECK(ts::Utc{time}.rtklib_time_string(3) == test.time_str);

                auto result = eph.compute(time);

                CHECK(doctest::Approx(result.position.x) == test.x);
                CHECK(doctest::Approx(result.position.y) == test.y);
                CHECK(doctest::Approx(result.position.z) == test.z);

                CHECK(doctest::Approx(result.velocity.x).epsilon(0.001) == test.vx);
                CHECK(doctest::Approx(result.velocity.y).epsilon(0.001) == test.vy);
                CHECK(doctest::Approx(result.velocity.z).epsilon(0.001) == test.vz);

                auto delta_time = 1e-3;
                auto time2 =
                    ts::Gst{ts::Gps{ts::Timestamp{gps_time.timestamp().seconds(), delta_time}}};
                auto result2 = eph.compute(time2);

                CHECK(doctest::Approx(result2.position.x) == test.x2);
                CHECK(doctest::Approx(result2.position.y) == test.y2);
                CHECK(doctest::Approx(result2.position.z) == test.z2);

                auto diff_vx = (result2.position.x - result.position.x) / delta_time;
                auto diff_vy = (result2.position.y - result.position.y) / delta_time;
                auto diff_vz = (result2.position.z - result.position.z) / delta_time;

                CHECK(doctest::Approx(diff_vx).epsilon(0.001) == test.vx);
                CHECK(doctest::Approx(diff_vy).epsilon(0.001) == test.vy);
                CHECK(doctest::Approx(diff_vz).epsilon(0.001) == test.vz);

                auto clock_bias = result.clock + result.relativistic_correction_brdc;
                CHECK(doctest::Approx(clock_bias) == test.clock_bias);

                count++;
            }
        }

        CHECK(count > 0);
    }
}
