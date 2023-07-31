#pragma once
#include "example.h"

namespace agnss_example {

enum class Format {
    XER,
};

void execute(const LocationServerOptions& location_server_options,
             const IdentityOptions& identity_options, const CellOptions& cell_options,
             const ModemOptions& modem_options, const OutputOptions& output_options, Format format);

}  // namespace agnss_example
