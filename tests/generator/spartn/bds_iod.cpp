#include <cstdint>
#include <doctest/doctest.h>

// BDS SF100 IODC formula: IODC = (iod_r15 * 512 / 720) % 240
// iod_r15 for B1I/B3I = 11 MSBs of toe (scale factor 512s)
static uint8_t bds_iodc(int64_t iod_r15) {
    return static_cast<uint8_t>(static_cast<uint64_t>(iod_r15) * 512 / 720 % 240);
}

TEST_CASE("BDS SF100 IODC: iod_r15=0 (toe=0s)") {
    CHECK(bds_iodc(0) == 0);
}

TEST_CASE("BDS SF100 IODC: iod_r15=2 (toe=1024s) -> 1024/720=1") {
    CHECK(bds_iodc(2) == 1);
}

TEST_CASE("BDS SF100 IODC: iod_r15=120 (toe=61440s) -> 61440/720=85") {
    CHECK(bds_iodc(120) == 85);
}

TEST_CASE("BDS SF100 IODC: iod_r15=1012 (toe=518144s) -> 518144/720=239 (max before wrap)") {
    CHECK(bds_iodc(1012) == 239);
}

TEST_CASE("BDS SF100 IODC: iod_r15=1013 (toe=518656s) -> wraps to 0") {
    CHECK(bds_iodc(1013) == 0);
}
