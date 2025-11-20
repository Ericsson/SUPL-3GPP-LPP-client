#include <core/error.hpp>
#include <core/generic.hpp>
#include <core/result.hpp>
#include <doctest/doctest.h>
#include <string>

static Error validate_input(int value) {
    if (value < 0) {
        return core::make_error(core::ErrorCode::InvalidArgument);
    }
    return ok();
}

static Error process_data(int value) {
    auto err = validate_input(value);
    if (!err) {
        return err;
    }
    return ok();
}

static Error allocate_memory(size_t size) {
    if (size > 1000000) {
        return core::make_error(core::ErrorCode::OutOfMemory);
    }
    return ok();
}

static Error process_with_try(int value) {
    TRY(validate_input(value));
    TRY(allocate_memory(100));
    return ok();
}

static Error nested_try(int value) {
    TRY(process_with_try(value));
    return ok();
}

TEST_CASE("Usage: validation pattern") {
    auto err1 = validate_input(10);
    CHECK(err1.ok());

    auto err2 = validate_input(-5);
    CHECK_FALSE(err2.ok());
    CHECK(err2.domain == ErrorDomain::Generic);
}

TEST_CASE("Usage: error propagation") {
    auto err1 = process_data(10);
    CHECK(err1.ok());

    auto err2 = process_data(-5);
    CHECK_FALSE(err2.ok());
}

TEST_CASE("Usage: TRY macro (Rust-style ?)") {
    auto err1 = process_with_try(10);
    CHECK(err1.ok());

    auto err2 = process_with_try(-5);
    CHECK_FALSE(err2.ok());
    CHECK(err2 == core::make_error(core::ErrorCode::InvalidArgument));

    auto err3 = nested_try(10);
    CHECK(err3.ok());
}

TEST_CASE("Usage: TRY_UNWRAP macro") {
    auto compute = [](int input) -> Result<int> {
        if (input < 0) {
            return err<int>(core::make_error(core::ErrorCode::InvalidArgument));
        }
        return ok(input * 2);
    };

    auto use_result = [&](int input) -> Error {
        int value = TRY_UNWRAP(compute(input));
        CHECK(value == input * 2);
        return ok();
    };

    auto err1 = use_result(10);
    CHECK(err1.ok());

    auto err2 = use_result(-5);
    CHECK_FALSE(err2.ok());
    CHECK(err2 == core::make_error(core::ErrorCode::InvalidArgument));
}

TEST_CASE("Usage: error comparison in conditionals") {
    auto specific_error = core::make_error(core::ErrorCode::InvalidArgument);

    if (specific_error == core::make_error(core::ErrorCode::InvalidArgument)) {
        CHECK(true);
    } else {
        CHECK(false);
    }

    if (specific_error != core::make_error(core::ErrorCode::OutOfMemory)) {
        CHECK(true);
    } else {
        CHECK(false);
    }
}
