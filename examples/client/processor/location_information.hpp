
#pragma once
#include <memory>

#include <lpp/location_information.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"

struct Program;
class LocationCollector : public streamline::Inspector<lpp::LocationInformation> {
public:
    LocationCollector(Program& program) : mProgram(program) {}

    const char* name() const NOEXCEPT override { return "LocationCollector"; }
    void inspect(streamline::System&, DataType const& location, uint64_t tag) NOEXCEPT override;

private:
    Program& mProgram;
};

class MetricsCollector : public streamline::Inspector<lpp::HaGnssMetrics> {
public:
    MetricsCollector(Program& program) : mProgram(program) {}

    const char* name() const NOEXCEPT override { return "MetricsCollector"; }
    void inspect(streamline::System&, DataType const& metrics, uint64_t tag) NOEXCEPT override;

private:
    Program& mProgram;
};

class LocationOutput : public streamline::Inspector<lpp::LocationInformation> {
public:
    LocationOutput(OutputConfig const& output) : mOutput(output) {}

    const char* name() const NOEXCEPT override { return "LocationOutput"; }
    void inspect(streamline::System&, DataType const& location, uint64_t tag) NOEXCEPT override;

private:
    OutputConfig const& mOutput;
};
