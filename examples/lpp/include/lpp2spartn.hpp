#include <streamline/inspector.hpp>

#include "lpp.hpp"

#ifdef INCLUDE_GENERATOR_SPARTN
#include <generator/spartn2/generator.hpp>

class Lpp2Spartn : public streamline::Inspector<LppMessage> {
public:
    Lpp2Spartn(OutputOptions const& options)
        : mGenerator(), mOptions(options) {}

    generator::spartn::Generator& generator() { return mGenerator; }

    void inspect(streamline::System&, DataType const& message) override;

private:
    generator::spartn::Generator mGenerator;
    OutputOptions const&          mOptions;
};
#endif
