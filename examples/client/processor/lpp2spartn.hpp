#pragma once
#include "../program_io.hpp"

#if !defined(INCLUDE_GENERATOR_SPARTN)
#error "INCLUDE_GENERATOR_SPARTN must be defined"
#endif

#include <generator/spartn2/generator.hpp>

#include "config.hpp"
#include "lpp.hpp"

class Lpp2Spartn : public streamline::Inspector<lpp::Message> {
public:
    Lpp2Spartn(ProgramOutput const& output, Lpp2SpartnConfig const& config);
    ~Lpp2Spartn() override;

    NODISCARD char const* name() const NOEXCEPT override { return "Lpp2Spartn"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) override;

    NODISCARD generator::spartn::Generator const* generator() const { return mGenerator.get(); }

private:
    std::unique_ptr<generator::spartn::Generator> mGenerator;

    ProgramOutput const&    mOutput;
    Lpp2SpartnConfig const& mConfig;
    uint64_t                mOutputTag;
};
