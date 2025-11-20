#include <core/error.hpp>
#include <core/generic.hpp>
#include <doctest/doctest.h>
#include <string>

TEST_CASE("Error ok state") {
    auto err = ok();

    CHECK(err.ok());
    CHECK(err);
    CHECK(err.domain == ErrorDomain::Generic);
    CHECK(err.code == 0);
    CHECK(err.system_errno == 0);
}

TEST_CASE("Error creation") {
    auto err = make_error(ErrorDomain::Scheduler, 42, 13);

    CHECK_FALSE(err.ok());
    CHECK_FALSE(err);
    CHECK(err.domain == ErrorDomain::Scheduler);
    CHECK(err.code == 42);
    CHECK(err.system_errno == 13);
}

TEST_CASE("Domain names") {
    CHECK(std::string(make_error(ErrorDomain::Generic, 1).domain_name()) == "Generic");
    CHECK(std::string(make_error(ErrorDomain::Scheduler, 1).domain_name()) == "Scheduler");
    CHECK(std::string(make_error(ErrorDomain::IO, 1).domain_name()) == "IO");
    CHECK(std::string(make_error(ErrorDomain::LPP, 1).domain_name()) == "LPP");
    CHECK(std::string(make_error(ErrorDomain::SUPL, 1).domain_name()) == "SUPL");
    CHECK(std::string(make_error(ErrorDomain::Format, 1).domain_name()) == "Format");
    CHECK(std::string(make_error(ErrorDomain::Modem, 1).domain_name()) == "Modem");
}

TEST_CASE("Generic error codes") {
    using namespace core;

    auto err1 = make_error(core::ErrorCode::OutOfMemory);
    CHECK(err1.domain == ErrorDomain::Generic);
    CHECK(err1.code == static_cast<ErrorCodeBase>(core::ErrorCode::OutOfMemory));
    CHECK_FALSE(err1.ok());

    auto err2 = make_error(ErrorCode::OutOfMemory, 12);
    CHECK(err2.system_errno == 12);

    auto err3 = make_error(ErrorCode::None);
    CHECK(err3.ok());
}

TEST_CASE("Generic error strings") {
    using namespace core;

    CHECK(std::string(error_to_string(ErrorCode::None)) == "None");
    CHECK(std::string(error_to_string(ErrorCode::OutOfMemory)) == "OutOfMemory");
}

TEST_CASE("Error propagation") {
    auto create_error = []() -> Error {
        return core::make_error(core::ErrorCode::OutOfMemory);
    };

    auto propagate_error = [&]() -> Error {
        auto err = create_error();
        if (!err) {
            return err;
        }
        return ok();
    };

    auto err = propagate_error();
    CHECK_FALSE(err.ok());
    CHECK(err.domain == ErrorDomain::Generic);
    CHECK(err.code == static_cast<ErrorCodeBase>(core::ErrorCode::OutOfMemory));
}
