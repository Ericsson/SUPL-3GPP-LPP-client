#pragma once
#include <core/core.hpp>

#include <memory>

struct LPP_Message;
struct ProvideAssistanceData_r9_IEs;
struct RequestLocationInformation_r9_IEs;
struct A_GNSS_ProvideAssistanceData;

namespace lpp {

namespace custom {
template <typename T>
struct Deleter {
    void operator()(T* ptr);
};
}  // namespace custom

using Message = std::unique_ptr<LPP_Message, custom::Deleter<LPP_Message>>;

void print(Message const& message);
void print(A_GNSS_ProvideAssistanceData* message);
void destroy(A_GNSS_ProvideAssistanceData* message);

bool is_request_capabilities(Message const& message);
bool is_request_location_information(Message const& message);
bool is_request_assistance_data(Message const& message);
bool is_provide_capabilities(Message const& message);
bool is_provide_location_information(Message const& message);
bool is_provide_assistance_data(Message const& message);

bool is_abort(Message const& message);
bool is_error(Message const& message);

ProvideAssistanceData_r9_IEs* get_provide_assistance_data(Message const& message);

class PeriodicSessionHandle;
bool get_periodic_session(ProvideAssistanceData_r9_IEs const& inner,
                          PeriodicSessionHandle*              session);

RequestLocationInformation_r9_IEs* get_request_location_information(Message const& message);

}  // namespace lpp
