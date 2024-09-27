#include <streamline/inspector.hpp>

#include "lpp.hpp"

#ifdef INCLUDE_GENERATOR_RTCM
#include <generator/rtcm/generator.hpp>

class Lpp2Rtcm : public streamline::Inspector<LppMessage> {
public:
    Lpp2Rtcm(generator::rtcm::Generator* generator, generator::rtcm::MessageFilter filter,
             bool print, OutputOptions const& options)
        : mGenerator(generator), mFilter(filter), mOptions(options), mPrint(print) {}

    void inspect(streamline::System&, DataType const& message) override;

private:
    generator::rtcm::Generator*    mGenerator;
    generator::rtcm::MessageFilter mFilter;
    OutputOptions const&           mOptions;
    bool                           mPrint;
};
#endif
