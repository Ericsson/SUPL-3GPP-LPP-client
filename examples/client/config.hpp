#pragma once
#include <lpp/assistance_data.hpp>
#include <supl/cell.hpp>

enum class OutputFormat {
    RTCM,
    SPARTN_OLD,
    SPARTN,
    XER,
    UPER,
    LPP_RTCM,
};

struct Config {
    lpp::RequestAssistanceData::Type assistance_data_type;
    supl::Cell                       cell;
    OutputFormat                     output_format;
};
