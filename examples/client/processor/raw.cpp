#include "raw.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(p, raw);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, raw)

void RawOutput::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    for (auto& output : mOutput.outputs) {
        if (!output.raw_support()) continue;
        if (!output.accept_tag(tag)) {
            XVERBOSEF(OUTPUT_PRINT_MODULE, "tag %llX not accepted", tag);
            continue;
        }
        XDEBUGF(OUTPUT_PRINT_MODULE, "raw: %zd bytes tag=%llX", message.data.size(), tag);

        ASSERT(output.stage, "stage is null");
        output.stage->write(OUTPUT_FORMAT_RAW, message.data.data(), message.data.size());
    }
}
