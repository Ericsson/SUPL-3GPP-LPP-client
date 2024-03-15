#pragma once
#include <lpp/types.hpp>

#include <memory>

struct LPP_Message;

namespace lpp {

namespace custom {
template <typename T>
struct Deleter {
    void operator()(T* ptr);
};
}  // namespace custom

using Message = std::unique_ptr<LPP_Message, custom::Deleter<LPP_Message>>;

}  // namespace lpp
