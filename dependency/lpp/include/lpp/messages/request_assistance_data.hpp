#pragma once
#include <lpp/message.hpp>
#include <lpp/periodic_session.hpp>
#include <supl/cell.hpp>

namespace lpp {
namespace messages {
struct RequestAssistanceData {
    supl::Cell            cell;
    PeriodicSessionHandle periodic_session;

    bool gps;
    bool glonass;
    bool galileo;
    bool bds;

    long delivery_amount;
    bool rtk_antenna_height;

    // The periodicity of each assistance data type. A value of 0 means that the assistance data is
    // not requested.
    long reference_time;
    long ionospheric_model;

    long rtk_observations;
    long rtk_residuals;
    long rtk_bias_information;
    long rtk_reference_station_info;

    long ssr_clock;
    long ssr_orbit;
    long ssr_code_bias;
    long ssr_phase_bias;
    long ssr_stec;
    long ssr_gridded;
    long ssr_ura;
    long ssr_correction_points;
};

struct RequestAssistanceData_Update {
    supl::Cell            cell;
    PeriodicSessionHandle periodic_session;
};

Message create_request_assistance_data(const RequestAssistanceData& request);
Message create_request_assistance_data(const RequestAssistanceData_Update& request);
}  // namespace messages
}  // namespace lpp
