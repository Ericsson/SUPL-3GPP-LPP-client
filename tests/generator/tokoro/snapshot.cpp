#include <doctest/doctest.h>
#include <generator/tokoro/generator.hpp>
#include <generator/tokoro/snapshot.hpp>
#include <msgpack/msgpack.hpp>
#include <test_utils.hpp>

#include <fstream>
#include <string>
#include <vector>

static bool load_msgpack(std::string const& filename, generator::tokoro::SnapshotInput& input) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) return false;

    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(ifs)),
                                std::istreambuf_iterator<char>());
    msgpack::Unpacker    unpacker(buffer.data(), buffer.size());
    return input.msgpack_unpack(unpacker);
}

static bool load_msgpack(std::string const& filename, generator::tokoro::SnapshotOutput& output) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) return false;

    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(ifs)),
                                std::istreambuf_iterator<char>());
    msgpack::Unpacker    unpacker(buffer.data(), buffer.size());
    return output.msgpack_unpack(unpacker);
}

static std::vector<std::string> find_input_files() {
    return test_utils::find_files_with_prefix_and_suffix(TEST_DATA_DIR "/tokoro",
                                                         "tokoro_snapshot_", ".msgpack");
}

TEST_CASE("Tokoro Snapshot") {
    auto input_files = find_input_files();
    REQUIRE(input_files.size() > 0);

    for (auto const& input_file : input_files) {
        CAPTURE(input_file);

        generator::tokoro::SnapshotInput input;
        REQUIRE(load_msgpack(input_file, input));

        auto output_file =
            input_file;  // convert tokoro_snapshot_XXXX.msgpack to tokoro_output_XXXX.msgpack
        output_file.replace(output_file.find("tokoro_snapshot_"), 16, "tokoro_output_");
        CAPTURE(output_file);
        REQUIRE(test_utils::file_exists(output_file));

        generator::tokoro::Generator gen;
        gen.load_snapshot(input);

        auto config = generator::tokoro::ReferenceStationConfig{input.config.itrf_position,
                                                                input.config.rtcm_position,
                                                                input.config.gps,
                                                                input.config.glo,
                                                                input.config.gal,
                                                                input.config.bds,
                                                                input.config.qzs};

        auto station = gen.define_reference_station(config);
        REQUIRE(station->generate(input.time));

        generator::tokoro::SnapshotOutput actual_output;
        generator::tokoro::extract_observations(station, actual_output);

        generator::tokoro::SnapshotOutput expected;
        REQUIRE(load_msgpack(output_file, expected));

        REQUIRE(actual_output.observations.size() == expected.observations.size());

        for (size_t i = 0; i < actual_output.observations.size(); ++i) {
            auto const& actual = actual_output.observations[i];
            auto const& expect = expected.observations[i];

            CAPTURE(i);
            CAPTURE(actual.gnss);
            CAPTURE(actual.prn);
            CAPTURE(actual.signal);

            CHECK(actual.pseudorange == doctest::Approx(expect.pseudorange).epsilon(1e-3));
            CHECK(actual.carrier_phase == doctest::Approx(expect.carrier_phase).epsilon(1e-4));
            CHECK(actual.doppler == doctest::Approx(expect.doppler).epsilon(1e-2));
        }
    }
}
