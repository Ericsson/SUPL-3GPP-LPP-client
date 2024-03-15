#pragma once
#include <loglet/loglet.hpp>
#include <supl/types.hpp>

#define LOGLET_CURRENT_MODULE "supl"

#define SCOPE_FUNCTION()                                                                           \
    DEBUGF("%s()", LOGLET_CURRENT_FUNCTION);                                                       \
    LOGLET_INDENT_SCOPE()

#define SCOPE_FUNCTIONF(fmt, ...)                                                                  \
    DEBUGF("%s(" fmt ")", LOGLET_CURRENT_FUNCTION, ##__VA_ARGS__);                                 \
    LOGLET_INDENT_SCOPE()

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

    DeferFinalizer(const DeferFinalizer&)            = delete;
    DeferFinalizer& operator=(const DeferFinalizer&) = delete;
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
#define SUPL_DEFER auto TOKENPASTE2(__deferred_lambda_call, __COUNTER__) = deferrer << [&]
