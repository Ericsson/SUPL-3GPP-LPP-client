#pragma once
#include <core/core.hpp>

#include <fstream>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
#include <maths/float3.hpp>
#include <time/tai.hpp>

namespace format {
namespace rinex {

enum class ObservationKind {
    Code,
    Phase,
    Doppler,
    SignalStrength,
};

struct ObservationType {
    ObservationKind kind;
    SignalId        signal_id;

    inline char kind_char() const {
        switch (kind) {
        case ObservationKind::Code: return 'C';
        case ObservationKind::Phase: return 'L';
        case ObservationKind::Doppler: return 'D';
        case ObservationKind::SignalStrength: return 'S';
        }
        return '?';
    }
};

struct Observation {
    ObservationType type;
    double          value;
};

class Builder {
public:
    explicit Builder(std::string path, double version) NOEXCEPT;
    ~Builder() NOEXCEPT = default;

    void epoch(ts::Tai const& time, std::vector<SatelliteId>& satellites);
    void observations(SatelliteId                                        id,
                      std::unordered_map<ObservationType, double> const& observations,
                      std::unordered_set<SignalId> const&                loss_signals);

    void set_antenna_position(Float3 const& position) NOEXCEPT { mApproxPosition = position; }
    void set_gps_support(bool support) NOEXCEPT { mGpsSupport = support; }
    void set_glo_support(bool support) NOEXCEPT { mGloSupport = support; }
    void set_gal_support(bool support) NOEXCEPT { mGalSupport = support; }
    void set_bds_support(bool support) NOEXCEPT { mBdsSupport = support; }

protected:
    void header_begin();
    void header_time_of_first_observation(ts::Tai const& time);
    void header_end();

    void header_comment(std::string const& comment);
    void header_observation_types();
    void header_observation_types_gnss(std::vector<ObservationType> const& types, char gnss_ch);

    void header_phase_shift_type(ObservationType const& type, char gnss_ch);
    void header_phase_shift();

    std::vector<ObservationType> const& observation_types(SatelliteId const& id) const;

    void generate_observation_order();

private:
    double      mVersion;
    std::string mProgram;
    std::string mRunBy;
    std::string mMarkerName;
    std::string mMarkerType;
    std::string mObserver;
    std::string mAgency;
    std::string mReceiverNumber;
    std::string mReceiverType;
    std::string mReceiverVersion;
    std::string mAntennaSerial;
    std::string mAntennaNumber;
    std::string mAntennaType;
    Float3      mApproxPosition;
    Float3      mAntennaDelta;  // H/E/N

    std::vector<ObservationType> mGpsTypeOrder;
    std::vector<ObservationType> mGloTypeOrder;
    std::vector<ObservationType> mGalTypeOrder;
    std::vector<ObservationType> mBdsTypeOrder;

    bool mGpsSupport = true;
    bool mGloSupport = true;
    bool mGalSupport = true;
    bool mBdsSupport = true;

    bool          mInitialized = false;
    std::string   mPath;
    std::ofstream mStream;
};

inline bool operator<(ObservationType const& a, ObservationType const& b) {
    if (a.signal_id.lpp_id() < b.signal_id.lpp_id()) {
        return true;
    } else if (a.signal_id.lpp_id() > b.signal_id.lpp_id()) {
        return false;
    } else {
        return a.kind < b.kind;
    }
}

inline bool operator==(ObservationType const& a, ObservationType const& b) {
    return a.kind == b.kind && a.signal_id == b.signal_id;
}

}  // namespace rinex
}  // namespace format

template <>
struct std::hash<format::rinex::ObservationType> {
    std::size_t operator()(format::rinex::ObservationType const& type) const {
        return std::hash<int>()(static_cast<int>(type.kind)) ^
               std::hash<SignalId>()(type.signal_id);
    }
};
