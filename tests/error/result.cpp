#include <core/error.hpp>
#include <core/generic.hpp>
#include <core/result.hpp>
#include <doctest/doctest.h>
#include <string>

TEST_CASE("Result basic") {
    Result<int> result = {42, ok()};

    CHECK(result.ok());
    CHECK(result);
    CHECK(result.value() == 42);
    CHECK(result.unwrap() == 42);
    CHECK(*result == 42);
}

TEST_CASE("Result with error") {
    Result<int> result = {0, core::make_error(core::ErrorCode::InvalidArgument)};

    CHECK_FALSE(result.ok());
    CHECK_FALSE(result);
    CHECK(result.error().code == static_cast<ErrorCodeBase>(core::ErrorCode::InvalidArgument));
}

TEST_CASE("Result helpers") {
    auto success = ok(42);
    CHECK(success.ok());
    CHECK(success.value() == 42);

    auto failure = err<int>(core::make_error(core::ErrorCode::OutOfMemory));
    CHECK_FALSE(failure.ok());
    CHECK(failure.error().code == static_cast<ErrorCodeBase>(core::ErrorCode::OutOfMemory));
}

TEST_CASE("Error error() method") {
    auto err = core::make_error(core::ErrorCode::InvalidArgument);
    CHECK(err.error() == err);
    CHECK(err.error().code == err.code);
}

TEST_CASE("TRY with Error") {
    auto validate = [](int value) -> Error {
        if (value < 0) {
            return core::make_error(core::ErrorCode::InvalidArgument);
        }
        return ok();
    };

    auto process = [&](int value) -> Error {
        TRY(validate(value));
        return ok();
    };

    CHECK(process(10).ok());
    CHECK_FALSE(process(-5).ok());
}

TEST_CASE("TRY with Result<T>") {
    auto get_value = [](int input) -> Result<int> {
        if (input < 0) {
            return err<int>(core::make_error(core::ErrorCode::InvalidArgument));
        }
        return ok(input * 2);
    };

    auto process = [&](int input) -> Error {
        int result = TRY(get_value(input));
        CHECK(result == input * 2);
        return ok();
    };

    CHECK(process(10).ok());
    CHECK_FALSE(process(-5).ok());
}

TEST_CASE("TRY_UNWRAP with Result<T>") {
    auto get_value = [](int input) -> Result<int> {
        if (input < 0) {
            return err<int>(core::make_error(core::ErrorCode::InvalidArgument));
        }
        return ok(input * 2);
    };

    auto process = [&](int input) -> Error {
        int value = TRY_UNWRAP(get_value(input));
        CHECK(value == input * 2);
        return ok();
    };

    CHECK(process(10).ok());
    CHECK_FALSE(process(-5).ok());
}

TEST_CASE("TRY chaining") {
    auto step1 = []() -> Error {
        return ok();
    };

    auto step2 = [](int input) -> Result<int> {
        return ok(input + 5);
    };

    auto step3 = [](int value) -> Result<int> {
        return ok(value + 10);
    };

    auto pipeline = [&](int input) -> Result<int> {
        TRY(step1());
        int v1 = TRY_UNWRAP(step2(input));
        int v2 = TRY_UNWRAP(step3(v1));
        return ok(v2);
    };

    auto r1 = pipeline(10);
    CHECK(r1.ok());
    CHECK(r1.value() == 25);
}

TEST_CASE("Result with complex types") {
    Result<std::string> result = {"hello", ok()};

    CHECK(result.ok());
    CHECK(result.value() == "hello");
    CHECK(*result == "hello");
}

TEST_CASE("Mixed TRY usage") {
    auto validate = [](int value) -> Error {
        if (value < 0) {
            return core::make_error(core::ErrorCode::InvalidArgument);
        }
        return ok();
    };

    auto compute = [](int value) -> Result<int> {
        if (value > 100) {
            return err<int>(core::make_error(core::ErrorCode::OutOfMemory));
        }
        return ok(value * 2);
    };

    auto process = [&](int value) -> Result<int> {
        TRY(validate(value));
        int result = TRY_UNWRAP(compute(value));
        return ok(result + 5);
    };

    auto r1 = process(10);
    CHECK(r1.ok());
    CHECK(r1.value() == 25);

    auto r2 = process(-5);
    CHECK_FALSE(r2.ok());
    CHECK(r2.error().code == static_cast<ErrorCodeBase>(core::ErrorCode::InvalidArgument));

    auto r3 = process(150);
    CHECK_FALSE(r3.ok());
    CHECK(r3.error().code == static_cast<ErrorCodeBase>(core::ErrorCode::OutOfMemory));
}
