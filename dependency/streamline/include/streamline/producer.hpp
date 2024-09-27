#pragma once
#include <streamline/system.hpp>

namespace streamline {
template <typename T>
class Producer {
public:
    using DataType = T;

    Producer(System& system) : mSystem(system) {}
    virtual ~Producer() = default;

    virtual void run() = 0;

protected:
    void produce(DataType&& data) { mSystem.push(std::move(data)); }

private:
    System& mSystem;
};
}  // namespace streamline
