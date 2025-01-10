#pragma once
#include <core/core.hpp>
#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "supl"

#include <memory>

// Defer Macro
// https://gist.github.com/win-t/125f9e75c0a0f4a74a951478d27ccb4f
template <typename F>
class DeferFinalizer final {
    F    func;
    bool moved;

public:
    DeferFinalizer(F&& f) : func(std::forward<F>(f)), moved(false) {}

    DeferFinalizer(DeferFinalizer&& other) : func(std::move(other.func)), moved(other.moved) {
        other.moved = true;
    }

    ~DeferFinalizer() {
        if (!moved) func();
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
