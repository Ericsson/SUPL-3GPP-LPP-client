#pragma once
#include <core/core.hpp>

#include <memory>

struct LPP_Message;
struct ProvideAssistanceData_r9_IEs;

namespace lpp {

namespace custom {
template <typename T>
struct Deleter {
    void operator()(T* ptr);
};
}  // namespace custom

using Message = std::unique_ptr<LPP_Message, custom::Deleter<LPP_Message>>;

bool is_request_capabilities(const Message& message);
bool is_request_location_information(const Message& message);
bool is_request_assistance_data(const Message& message);
bool is_provide_capabilities(const Message& message);
bool is_provide_location_information(const Message& message);
bool is_provide_assistance_data(const Message& message);

bool is_abort(const Message& message);
bool is_error(const Message& message);

ProvideAssistanceData_r9_IEs* get_provide_assistance_data(const Message& message);

class PeriodicSessionHandle;
bool get_periodic_session(const ProvideAssistanceData_r9_IEs& inner, PeriodicSessionHandle* session);

}  // namespace lpp
