#pragma once
#include "example.h"

namespace osr_example {

enum class Format {
    RTCM,
    RG2,
    XER,
};

enum class MsmType {
    ANY,
    MSM4,
    MSM5,
    MSM7,
};

void execute(const LocationServerOptions& location_server_options,
             const IdentityOptions& identity_options, const CellOptions& cell_options,
             const ModemOptions& modem_options, const OutputOptions& output_options, Format format,
             MsmType msm_type);

}  // namespace osr_example
