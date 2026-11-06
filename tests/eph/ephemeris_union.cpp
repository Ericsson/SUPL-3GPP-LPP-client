#include <doctest/doctest.h>
#include <ephemeris/ephemeris.hpp>
#include <msgpack/msgpack.hpp>
#include <time/glo.hpp>
#include <time/utc.hpp>

#include <vector>

static ephemeris::GloEphemeris make_glo() {
    ephemeris::GloEphemeris eph{};
    eph.slot_number      = 5;
    eph.frequency_number = 1;
    eph.reference_time   = ts::Glo(ts::Utc::from_date_time(2026, 6, 6, 0, 15, 0.0));
    eph.position         = Float3{10732.3671875, -11441.13037109, 19524.78808594};
    eph.velocity         = Float3{-2.112960815430, -1.364078521729, 0.2336502075195};
    eph.acceleration     = Float3{9.3e-10, 0.0, -2.79e-9};
    eph.tau_n            = -2.391310408711e-05;
    eph.gamma_n          = 9.094947017729e-13;
    eph.lpp_iod          = 13;
    return eph;
}

TEST_CASE("Ephemeris - GLONASS") {
    auto glo = make_glo();
    auto eph = ephemeris::Ephemeris(glo);
    CHECK(eph.mType == ephemeris::Ephemeris::Type::GLO);
    CHECK(eph.iod() == 13);

    auto time = ts::Tai(glo.reference_time + 600.0);
    CHECK(eph.is_valid(time));
    CHECK_FALSE(eph.is_valid(ts::Tai(glo.reference_time + 3600.0)));

    // broadcast clock: -tau_n + gamma_n * (t - tb), relativity included
    CHECK(eph.clock_bias(time) == doctest::Approx(-glo.tau_n + glo.gamma_n * 600.0));

    auto result = eph.compute(time);
    CHECK(result.position.length() == doctest::Approx(25.5e6).epsilon(0.05));
    CHECK(result.clock + eph.relativistic_correction(result.position, result.velocity) ==
          doctest::Approx(-glo.tau_n + glo.gamma_n * 600.0));

    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);
    eph.msgpack_pack(packer);
    msgpack::Unpacker    unpacker(buffer.data(), buffer.size());
    ephemeris::Ephemeris copy;
    REQUIRE(copy.msgpack_unpack(unpacker));
    CHECK(copy.mType == ephemeris::Ephemeris::Type::GLO);
    CHECK(copy.glo_ephemeris.slot_number == 5);
    CHECK(copy.glo_ephemeris.frequency_number == 1);
}
