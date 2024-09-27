#include "lpp2rtcm.hpp"

#ifdef INCLUDE_GENERATOR_RTCM

void Lpp2Rtcm::inspect(streamline::System&, DataType const& message) {
    auto messages = mGenerator->generate(message.get(), mFilter);

    if (mPrint) {
        size_t length = 0;
        for (auto& submessage : messages) {
            length += submessage.data().size();
        }

        printf("RTCM: %4zu bytes | ", length);
        for (auto& submessage : messages) {
            printf("%4i ", submessage.id());
        }
        printf("\n");
    }

    for (auto& submessage : messages) {
        auto buffer = submessage.data().data();
        auto size   = submessage.data().size();

        // TODO(ewasjon): These message should be passed back into the system
        for (auto const& output : mOptions.outputs) {
            if ((output.format & OUTPUT_FORMAT_RTCM) != 0) {
                output.interface->write(buffer, size);
            }
        }
    }
}

#endif
