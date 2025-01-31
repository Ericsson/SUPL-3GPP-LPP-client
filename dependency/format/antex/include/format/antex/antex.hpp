#pragma once
#include <core/core.hpp>

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
#include <maths/float3.hpp>
#include <time/gps.hpp>
#include <time/tai.hpp>

namespace format {
namespace antex {

enum class SatelliteSystem {
    GPS,
    GLONASS,
    GALILEO,
    COMPASS,
    QZSS,
    SBAS,
    MIXED,
};

enum class PcvType {
    ABSOLUTE,
    RELATIVE,
};

struct PhaseVariation {
    double value;
};

struct AntexHeader {
    std::string version;
    std::string system;
    std::string pcv_type;
    std::string refant_type;
    std::string refant_serial;
};

struct Frequency {
    FrequencyType                    type;
    Float3                           eccentricities;
    double                           dazi{0.0f};
    double                           zen1{0.0f};
    double                           zen2{0.0f};
    double                           dzen{0.0f};
    std::vector<double>              no_azimuth;
    std::vector<std::vector<double>> azimuths;

    bool phase_variation(double azimuth, double elevation, PhaseVariation& phase_variation) const;
};

struct FrequencyRms {
    FrequencyType type;
    double        rms{0.0};
};

struct Antenna {
    SatelliteId id;
    double      dazi{0.0f};
    double      zen1{0.0f};
    double      zen2{0.0f};
    double      dzen{0.0f};
    int64_t     frequency_count{0};
    ts::Gps     valid_from;
    ts::Gps     valid_until;
    bool        valid_from_set{false};
    bool        valid_until_set{false};

    std::unordered_map<FrequencyType, std::unique_ptr<Frequency>> frequencies;

    bool phase_variation(SignalId const& signal_id, double azimuth, double elevation,
                         PhaseVariation& phase_variation) const;
};

class Antex {
public:
    AntexHeader                                                            header;
    std::unordered_map<SatelliteId, std::vector<std::unique_ptr<Antenna>>> antennas;

    bool phase_variation(SatelliteId const& satellite_id, SignalId const& signal_id,
                         ts::Tai const& time, double azimuth, double elevation,
                         PhaseVariation& phase_variation) const;

    static std::unique_ptr<Antex> from_file(std::string const& path);
    static std::unique_ptr<Antex> from_string(std::string const& data);
};

}  // namespace antex
}  // namespace format
