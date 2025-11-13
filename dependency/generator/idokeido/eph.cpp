#include "eph.hpp"

#include <loglet/loglet.hpp>

#include <fstream>
#include <ostream>

LOGLET_MODULE2(idokeido, eph);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, eph)

namespace idokeido {

struct CacheHeader {
    uint32_t magic   = 0x45504845;  // 'EPHE' in hex
    uint32_t version = 1;
    uint32_t gps_count;
    uint32_t gal_count;
    uint32_t bds_count;
    uint64_t timestamp;  // Cache creation time for validation
};

void EphemerisEngine::load_or_create_cache(std::string const& file_path) NOEXCEPT {
    FUNCTION_SCOPE();
    mGpsEphemeris.clear();
    mGalEphemeris.clear();
    mBdsEphemeris.clear();

    VERBOSEF("loading from cache: %s", file_path.c_str());
    mCacheFile = std::unique_ptr<std::string>(new std::string(file_path));

    std::ifstream file(file_path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        WARNF("failed to open cache file: %s", file_path.c_str());
        return;
    }

    CacheHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.magic != 0x45504845) {
        WARNF("invalid cache file format: %s", file_path.c_str());
        return;
    }

    if (header.version != 1) {
        WARNF("unsupported cache version: %u", header.version);
        return;
    }

    // Load GPS ephemeris
    for (uint32_t i = 0; i < header.gps_count; ++i) {
        SatelliteId sat_id;
        file.read(reinterpret_cast<char*>(&sat_id), sizeof(sat_id));

        ephemeris::GpsEphemeris eph;
        file.read(reinterpret_cast<char*>(&eph), sizeof(eph));

        mGpsEphemeris[sat_id].push_back(eph);
    }

    // Load Galileo ephemeris
    for (uint32_t i = 0; i < header.gal_count; ++i) {
        SatelliteId sat_id;
        file.read(reinterpret_cast<char*>(&sat_id), sizeof(sat_id));

        ephemeris::GalEphemeris eph;
        file.read(reinterpret_cast<char*>(&eph), sizeof(eph));

        mGalEphemeris[sat_id].push_back(eph);
    }

    // Load BDS ephemeris
    for (uint32_t i = 0; i < header.bds_count; ++i) {
        SatelliteId sat_id;
        file.read(reinterpret_cast<char*>(&sat_id), sizeof(sat_id));

        ephemeris::BdsEphemeris eph;
        file.read(reinterpret_cast<char*>(&eph), sizeof(eph));

        mBdsEphemeris[sat_id].push_back(eph);
    }

    VERBOSEF("loaded %u GPS, %u GAL, %u BDS ephemeris from cache", header.gps_count,
             header.gal_count, header.bds_count);
}

// Add this method to save cache
void EphemerisEngine::save_cache() NOEXCEPT {
    FUNCTION_SCOPE();
    if (!mCacheFile) return;

    std::ofstream file(*mCacheFile, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        WARNF("failed to open cache file for writing: %s", mCacheFile->c_str());
        return;
    }

    // Count total ephemeris
    uint32_t gps_count = 0, gal_count = 0, bds_count = 0;
    for (auto const& pair : mGpsEphemeris)
        gps_count += pair.second.size();
    for (auto const& pair : mGalEphemeris)
        gal_count += pair.second.size();
    for (auto const& pair : mBdsEphemeris)
        bds_count += pair.second.size();

    // Write header
    CacheHeader header;
    header.gps_count = gps_count;
    header.gal_count = gal_count;
    header.bds_count = bds_count;
    header.timestamp = static_cast<uint64_t>(time(nullptr));
    file.write(reinterpret_cast<char const*>(&header), sizeof(header));

    // Write GPS ephemeris
    for (auto const& sat_pair : mGpsEphemeris) {
        for (auto const& eph : sat_pair.second) {
            file.write(reinterpret_cast<char const*>(&sat_pair.first), sizeof(sat_pair.first));
            file.write(reinterpret_cast<char const*>(&eph), sizeof(eph));
        }
    }

    // Write Galileo ephemeris
    for (auto const& sat_pair : mGalEphemeris) {
        for (auto const& eph : sat_pair.second) {
            file.write(reinterpret_cast<char const*>(&sat_pair.first), sizeof(sat_pair.first));
            file.write(reinterpret_cast<char const*>(&eph), sizeof(eph));
        }
    }

    // Write BDS ephemeris
    for (auto const& sat_pair : mBdsEphemeris) {
        for (auto const& eph : sat_pair.second) {
            file.write(reinterpret_cast<char const*>(&sat_pair.first), sizeof(sat_pair.first));
            file.write(reinterpret_cast<char const*>(&eph), sizeof(eph));
        }
    }
}

void EphemerisEngine::add(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();
    auto satellite_id = SatelliteId::from_gps_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: GPS %d", ephemeris.prn);
        return;
    }

    auto& list = mGpsEphemeris[satellite_id];

    // Check if the ephemeris is already in the list
    for (auto& eph : list) {
        if (eph.match(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
            return;
        }
    }

    // Remove the oldest ephemeris if the list is full
    if (list.size() >= 10) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    std::sort(list.begin(), list.end(),
              [](ephemeris::GpsEphemeris const& a, ephemeris::GpsEphemeris const& b) {
                  return a.compare(b);
              });

    DEBUGF("ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
    save_cache();
}

void EphemerisEngine::add(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();
    auto satellite_id = SatelliteId::from_gal_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: GAL %d", ephemeris.prn);
        return;
    }

    auto& list = mGalEphemeris[satellite_id];

    // Check if the ephemeris is already in the list
    for (auto& eph : list) {
        if (eph.match(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
            return;
        }
    }

    // Remove the oldest ephemeris if the list is full
    if (list.size() >= 10) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    std::sort(list.begin(), list.end(),
              [](ephemeris::GalEphemeris const& a, ephemeris::GalEphemeris const& b) {
                  return a.compare(b);
              });

    DEBUGF("ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
    save_cache();
}

void EphemerisEngine::add(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();
    auto satellite_id = SatelliteId::from_bds_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: BDS %d", ephemeris.prn);
        return;
    }

    auto& list = mBdsEphemeris[satellite_id];

    // Check if the ephemeris is already in the list
    for (auto& eph : list) {
        if (eph.match(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
            return;
        }
    }

    // Remove the oldest ephemeris if the list is full
    if (list.size() >= 10) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    std::sort(list.begin(), list.end(),
              [](ephemeris::BdsEphemeris const& a, ephemeris::BdsEphemeris const& b) {
                  return a.compare(b);
              });

    DEBUGF("ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
    save_cache();
}

ephemeris::GpsEphemeris const* EphemerisEngine::find_gps(SatelliteId    satellite_id,
                                                         ts::Tai const& time) const NOEXCEPT {
    FUNCTION_SCOPE();
    auto it = mGpsEphemeris.find(satellite_id);
    if (it == mGpsEphemeris.end()) return nullptr;
    auto& list = it->second;

    auto gps_time = ts::Gps(time);
    VERBOSEF("searching: %s", satellite_id.name());
    for (auto& ephemeris : list) {
        VERBOSEF("  %4u %8.0f %8.0f | %4u %4u %4u |%s", ephemeris.week_number, ephemeris.toe,
                 ephemeris.toc, ephemeris.lpp_iod, ephemeris.iode, ephemeris.iodc,
                 ephemeris.is_valid(gps_time) ? " [time]" : "");
        if (!ephemeris.is_valid(gps_time)) continue;
        VERBOSEF("found: %s", satellite_id.name());
        return &ephemeris;
    }

    return nullptr;
}

ephemeris::GalEphemeris const* EphemerisEngine::find_gal(SatelliteId    satellite_id,
                                                         ts::Tai const& time) const NOEXCEPT {
    FUNCTION_SCOPE();
    auto it = mGalEphemeris.find(satellite_id);
    if (it == mGalEphemeris.end()) return nullptr;
    auto& list = it->second;

    auto gal_time = ts::Gst(time);
    VERBOSEF("searching: %s", satellite_id.name());
    for (auto& ephemeris : list) {
        VERBOSEF("  %4u %8.0f %8.0f | %4u %4u |%s", ephemeris.week_number, ephemeris.toe,
                 ephemeris.toc, ephemeris.lpp_iod, ephemeris.iod_nav,
                 ephemeris.is_valid(gal_time) ? " [time]" : "");
        if (!ephemeris.is_valid(gal_time)) continue;
        VERBOSEF("found: %s", satellite_id.name());
        return &ephemeris;
    }

    return nullptr;
}

ephemeris::BdsEphemeris const* EphemerisEngine::find_bds(SatelliteId    satellite_id,
                                                         ts::Tai const& time) const NOEXCEPT {
    FUNCTION_SCOPE();
    auto it = mBdsEphemeris.find(satellite_id);
    if (it == mBdsEphemeris.end()) return nullptr;
    auto& list = it->second;

    auto bds_time = ts::Bdt(time);
    VERBOSEF("searching: %s", satellite_id.name());
    for (auto& ephemeris : list) {
        VERBOSEF("  %4u %8.0f %8.0f | %4u %4u |%s", ephemeris.week_number, ephemeris.toe,
                 ephemeris.toc, ephemeris.lpp_iod, ephemeris.iodc,
                 ephemeris.is_valid(bds_time) ? " [time]" : "");
        if (!ephemeris.is_valid(bds_time)) continue;
        VERBOSEF("found: %s", satellite_id.name());
        return &ephemeris;
    }

    return nullptr;
}

bool EphemerisEngine::evaluate(SatelliteId satellite_id, ts::Tai const& time,
                               RelativisticModel relativistic_model,
                               Satellite&        result) const NOEXCEPT {
    FUNCTION_SCOPE();
    if (satellite_id.is_gps()) {
        auto eph = find_gps(satellite_id, time);
        if (!eph) return false;
        result = evaluate_gps(satellite_id, time, relativistic_model, *eph);
        return true;
    }
    if (satellite_id.is_galileo()) {
        auto eph = find_gal(satellite_id, time);
        if (!eph) return false;
        result = evaluate_gal(satellite_id, time, relativistic_model, *eph);
        return true;
    }
    if (satellite_id.is_beidou()) {
        auto eph = find_bds(satellite_id, time);
        if (!eph) return false;
        result = evaluate_bds(satellite_id, time, relativistic_model, *eph);
        return true;
    }

    return false;
}

bool EphemerisEngine::find(SatelliteId satellite_id, ts::Tai const& time,
                           ephemeris::Ephemeris& ephemeris) const NOEXCEPT {
    FUNCTION_SCOPE();
    if (satellite_id.is_gps()) {
        auto eph = find_gps(satellite_id, time);
        if (!eph) return false;
        ephemeris = ephemeris::Ephemeris{*eph};
        return true;
    }
    if (satellite_id.is_galileo()) {
        auto eph = find_gal(satellite_id, time);
        if (!eph) return false;
        ephemeris = ephemeris::Ephemeris{*eph};
        return true;
    }
    if (satellite_id.is_beidou()) {
        auto eph = find_bds(satellite_id, time);
        if (!eph) return false;
        ephemeris = ephemeris::Ephemeris{*eph};
        return true;
    }

    return false;
}

EphemerisEngine::Satellite
EphemerisEngine::evaluate_gps(SatelliteId satellite_id, ts::Tai const& time,
                              RelativisticModel              relativistic_model,
                              ephemeris::GpsEphemeris const& eph) const NOEXCEPT {
    FUNCTION_SCOPE();

    auto gps_time    = ts::Gps(time);
    auto result      = eph.compute(gps_time);
    auto group_delay = eph.calculate_group_delay();

    Scalar rc = 0.0;
    if (relativistic_model == RelativisticModel::Broadcast) {
        rc = result.relativistic_correction_brdc;
    } else if (relativistic_model == RelativisticModel::Dotrv) {
        rc = result.relativistic_correction_dotrv;
    }

    return {.id          = satellite_id,
            .position    = {result.position.x, result.position.y, result.position.z},
            .velocity    = {result.velocity.x, result.velocity.y, result.velocity.z},
            .clock       = result.clock + rc,
            .group_delay = group_delay};
}

EphemerisEngine::Satellite
EphemerisEngine::evaluate_gal(SatelliteId, ts::Tai const&, RelativisticModel,
                              ephemeris::GalEphemeris const&) const NOEXCEPT {
    FUNCTION_SCOPE();
    TODOF("implement EphemerisEngine::evaluate_gal()");
    return {};
}

EphemerisEngine::Satellite
EphemerisEngine::evaluate_bds(SatelliteId, ts::Tai const&, RelativisticModel,
                              ephemeris::BdsEphemeris const&) const NOEXCEPT {
    FUNCTION_SCOPE();
    TODOF("implement EphemerisEngine::evaluate_gal()");
    return {};
}

bool EphemerisEngine::clock_bias(SatelliteId satellite_id, ts::Tai const& time,
                                 Scalar& clock_bias) const NOEXCEPT {
    FUNCTION_SCOPE();
    if (satellite_id.is_gps()) {
        auto eph = find_gps(satellite_id, time);
        if (eph) {
            auto gps_time = ts::Gps(time);
            clock_bias    = eph->calculate_clock_bias(gps_time);
            return true;
        }

        return false;
    }
    if (satellite_id.is_galileo()) {
        auto eph = find_gal(satellite_id, time);
        if (eph) {
            TODOF("implement EphemerisEngine::clock_bias()");
        }

        return false;
    }
    if (satellite_id.is_beidou()) {
        auto eph = find_bds(satellite_id, time);
        if (eph) {
            TODOF("implement EphemerisEngine::clock_bias()");
        }

        return false;
    }

    return false;
}

}  // namespace idokeido
