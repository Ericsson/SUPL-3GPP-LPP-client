#include <doctest/doctest.h>
#include <format/antex/antex.hpp>

TEST_CASE("ANTEX parser - from_string") {
    auto antex = format::antex::Antex::from_string("     1.4            ANTEX VERSION / SYST\n");
    CHECK(antex != nullptr);
}
