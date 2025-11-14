#pragma once
#include <streamline/consumer.hpp>
#include <streamline/system.hpp>

LOGLET_MODULE_FORWARD_REF(streamline);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(streamline)

namespace streamline {
template <typename Input, typename Output>
class Pipeline : public Consumer<Input> {
public:
    using InputType  = Input;
    using OutputType = Output;

    Pipeline()          = default;
    virtual ~Pipeline() = default;

    virtual OutputType process(System&, InputType&& input) = 0;
    virtual void       consume(System& system, InputType&& input) override {
        VERBOSEF("pipeline %s -> %s", typeid(InputType).name(), typeid(OutputType).name());
        LOGLET_INDENT_SCOPE(loglet::Level::Verbose);
        auto output = process(system, std::move(input));
        system.push(std::move(output));
    }
};
}  // namespace streamline

#undef LOGLET_CURRENT_MODULE
