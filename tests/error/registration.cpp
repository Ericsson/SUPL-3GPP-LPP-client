#include <core/error.hpp>
#include <doctest/doctest.h>
#include <string>

namespace test_domain {

enum class ErrorCode : uint16_t {
    None       = 0,
    TestError1 = 1,
    TestError2 = 2,
};

char const* error_to_string(ErrorCodeBase code) {
    switch (static_cast<ErrorCode>(code)) {
    case ErrorCode::None: return "None";
    case ErrorCode::TestError1: return "TestError1";
    case ErrorCode::TestError2: return "TestError2";
    }
    return "Unknown";
}

}  // namespace test_domain

// Register the test domain (using Scheduler domain for testing)
REGISTER_ERROR_DOMAIN(ErrorDomain::Scheduler, test_domain::error_to_string);

TEST_CASE("Error domain registration") {
    auto err = make_error(ErrorDomain::Scheduler, 1);

    CHECK(err.domain == ErrorDomain::Scheduler);
    CHECK(err.code == 1);
    CHECK(std::string(err.domain_name()) == "Scheduler");
    CHECK(std::string(err.code_name()) == "TestError1");
}

TEST_CASE("Unregistered domain returns nullptr") {
    auto err = make_error(ErrorDomain::IO, 1);

    CHECK(err.code_name() == nullptr);
}
