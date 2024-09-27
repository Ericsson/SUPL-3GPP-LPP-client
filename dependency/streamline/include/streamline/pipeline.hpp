#pragma once
#include <streamline/consumer.hpp>
#include <streamline/system.hpp>

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
        XVERBOSEF("smtl", "pipeline %s -> %s", typeid(InputType).name(), typeid(OutputType).name());
        LOGLET_XINDENT_SCOPE("smtl", loglet::Level::Verbose);
        auto output = process(system, std::move(input));
        system.push(std::move(output));
    }
};
}  // namespace streamline
