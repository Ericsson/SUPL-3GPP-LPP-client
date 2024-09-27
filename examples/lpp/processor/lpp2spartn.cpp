#include "lpp2spartn.hpp"

#ifdef INCLUDE_GENERATOR_SPARTN

void Lpp2Spartn::inspect(streamline::System&, DataType const& message) {
    auto messages = mGenerator.generate(message.get());
    for (auto& msg : messages) {
        auto data = msg.build();

        for (auto& output : mOptions.outputs) {
            if ((output.format & OUTPUT_FORMAT_SPARTN) != 0) {
                output.interface->write(data.data(), data.size());
            }
        }
    }
}

#endif
