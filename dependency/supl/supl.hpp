#pragma once
#include <core/core.hpp>
#include <loglet/loglet.hpp>

#include <memory>

// Defer Macro
// https://gist.github.com/win-t/125f9e75c0a0f4a74a951478d27ccb4f
template <typename F>
class DeferFinalizer final {
    F    mFunc;
    bool mMoved;

public:
    DeferFinalizer(F&& f) : mFunc(std::forward<F>(f)), mMoved(false) {}

    DeferFinalizer(DeferFinalizer&& other) : mFunc(std::move(other.mFunc)), mMoved(other.mMoved) {
        other.mMoved = true;
    }

    ~DeferFinalizer() {
        if (!mMoved) mFunc();
    }

    DeferFinalizer(DeferFinalizer const&)            = delete;
    DeferFinalizer& operator=(DeferFinalizer const&) = delete;
    DeferFinalizer& operator=(DeferFinalizer&&)      = delete;
};

struct {
    template <typename F>
    DeferFinalizer<F> operator<<(F&& f) {
        return DeferFinalizer<F>(std::forward<F>(f));
    }
} deferrer;

#define TOKENPASTE(x, y) x##y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define SUPL_DEFER auto TOKENPASTE2(deferred_lambda_call, __COUNTER__) = deferrer << [&]
