#include "obs.hpp"

#include <loglet/loglet.hpp>
#include <time/gps.hpp>
#include <time/utc.hpp>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <unordered_map>

LOGLET_MODULE2(format, rinex_obs);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(format, rinex_obs)

namespace format {
namespace rinex {

// Map RINEX 2-char signal code + system char to SignalId
static SignalId signal_from_rinex(char sys, std::string const& code) {
    if (sys == 'G') {
        if (code == "1C") return SignalId::GPS_L1_CA;
        if (code == "1P") return SignalId::GPS_L1_P;
        if (code == "1W") return SignalId::GPS_L1_Z_TRACKING;
        if (code == "2C") return SignalId::GPS_L2_C_A;
        if (code == "2P") return SignalId::GPS_L2_P;
        if (code == "2W") return SignalId::GPS_L2_Z_TRACKING;
        if (code == "2S") return SignalId::GPS_L2_L2C_M;
        if (code == "2L") return SignalId::GPS_L2_L2C_L;
        if (code == "2X") return SignalId::GPS_L2_L2C_M_L;
        if (code == "5I") return SignalId::GPS_L5_I;
        if (code == "5Q") return SignalId::GPS_L5_Q;
        if (code == "5X") return SignalId::GPS_L5_I_Q;
        if (code == "1S") return SignalId::GPS_L1_L1C_D;
        if (code == "1L") return SignalId::GPS_L1_L1C_P;
        if (code == "1X") return SignalId::GPS_L1_L1C_D_P;
    } else if (sys == 'R') {
        if (code == "1C") return SignalId::GLONASS_G1_CA;
        if (code == "2C") return SignalId::GLONASS_G2_CA;
        if (code == "1P") return SignalId::GLONASS_G1_P;
        if (code == "2P") return SignalId::GLONASS_G2_P;
    } else if (sys == 'E') {
        if (code == "1C") return SignalId::GALILEO_E1_C_NO_DATA;
        if (code == "1A") return SignalId::GALILEO_E1_A;
        if (code == "1B") return SignalId::GALILEO_E1_B_I_NAV_OS_CS_SOL;
        if (code == "1X") return SignalId::GALILEO_E1_B_C;
        if (code == "5I") return SignalId::GALILEO_E5A_I;
        if (code == "5Q") return SignalId::GALILEO_E5A_Q;
        if (code == "5X") return SignalId::GALILEO_E5A_I_Q;
        if (code == "7I") return SignalId::GALILEO_E5B_I;
        if (code == "7Q") return SignalId::GALILEO_E5B_Q;
        if (code == "7X") return SignalId::GALILEO_E5B_I_Q;
        if (code == "8I") return SignalId::GALILEO_E5_A_B_I;
        if (code == "8Q") return SignalId::GALILEO_E5_A_B_Q;
        if (code == "8X") return SignalId::GALILEO_E5_A_B_I_Q;
    } else if (sys == 'C') {
        if (code == "2I") return SignalId::BEIDOU_B1_I;
        if (code == "2Q") return SignalId::BEIDOU_B1_Q;
        if (code == "2X") return SignalId::BEIDOU_B1_I_Q;
        if (code == "6I") return SignalId::BEIDOU_B3_I;
        if (code == "6Q") return SignalId::BEIDOU_B3_Q;
        if (code == "6X") return SignalId::BEIDOU_B3_I_Q;
        if (code == "7I") return SignalId::BEIDOU_B2_I;
        if (code == "7Q") return SignalId::BEIDOU_B2_Q;
        if (code == "7X") return SignalId::BEIDOU_B2_I_Q;
    }
    return SignalId{};
}

// Parse observation types from header line "G    4 C1C L1C D1C S1C"
struct ObsTypeList {
    std::vector<std::pair<char, std::string>> types;  // (kind_char, signal_code)
};

static std::unordered_map<char, ObsTypeList> parse_obs_types(std::ifstream& f,
                                                             std::string&   first_line) {
    std::unordered_map<char, ObsTypeList> result;
    std::string                           line;

    while (std::getline(f, line)) {
        if (line.find("END OF HEADER") != std::string::npos) {
            first_line = "";
            return result;
        }

        if (line.size() >= 60 && line.substr(60).find("SYS / # / OBS TYPES") != std::string::npos) {
            char sys   = line[0];
            int  count = 0;
            std::sscanf(line.c_str() + 1, "%3d", &count);

            ObsTypeList list;
            // Types start at col 7, each is 4 chars wide; max 13 per line
            for (int i = 0; i < count; ++i) {
                if (i > 0 && i % 13 == 0) {
                    // Continuation line
                    if (!std::getline(f, line)) break;
                }
                int col = 7 + (i % 13) * 4;
                if (col + 3 > static_cast<int>(line.size())) break;
                char        kind = line[static_cast<size_t>(col)];
                std::string sig  = line.substr(static_cast<size_t>(col + 1), 2);
                list.types.push_back({kind, sig});
            }
            result[sys] = std::move(list);
        }
    }
    return result;
}

bool parse_obs(std::string const&                          path,
               std::function<void(ObsEpoch const&)> const& callback) NOEXCEPT {
    std::ifstream f(path);
    if (!f.is_open()) {
        ERRORF("failed to open obs file: %s", path.c_str());
        return false;
    }

    std::string first_line;
    auto        obs_types = parse_obs_types(f, first_line);

    long        epoch_count = 0;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] != '>') continue;

        // Parse epoch header: > YYYY MM DD HH MM SS.SSSSSSS  flag  num_sats
        int    year, month, day, hour, min, flag, num_sats;
        double sec;
        if (std::sscanf(line.c_str() + 1, " %4d %2d %2d %2d %2d %10lf %3d %3d", &year, &month, &day,
                        &hour, &min, &sec, &flag, &num_sats) < 8)
            continue;

        if (flag != 0) {
            // Skip special epochs (cycle slip, header, etc.)
            for (int i = 0; i < num_sats; ++i)
                std::getline(f, line);
            continue;
        }

        auto epoch_time = ts::Tai{ts::Gps::from_ymdhms(year, month, day, hour, min, sec)};

        ObsEpoch epoch;
        epoch.time = epoch_time;

        for (int s = 0; s < num_sats; ++s) {
            if (!std::getline(f, line)) break;
            if (line.size() < 3) continue;

            auto sv_id = SatelliteId::from_string(line.substr(0, 3));
            if (!sv_id.is_valid()) continue;

            char sys = line[0];
            auto it  = obs_types.find(sys);
            if (it == obs_types.end()) continue;

            auto const& types = it->second.types;

            // Collect all observations for this satellite, then emit one per signal
            // Group by signal code
            struct SigObs {
                double pseudo = 0.0, phase = 0.0, doppler = 0.0, snr = 0.0;
                bool   lli        = false;
                bool   has_pseudo = false, has_phase = false;
            };
            std::unordered_map<std::string, SigObs> sig_obs;

            for (size_t i = 0; i < types.size(); ++i) {
                size_t col = 3 + i * 16;
                if (col + 14 > line.size()) break;

                auto val_str = line.substr(col, 14);
                // trim
                size_t start = val_str.find_first_not_of(' ');
                if (start == std::string::npos) continue;

                double val = 0.0;
                try {
                    val = std::stod(val_str.substr(start));
                } catch (...) {
                    continue;
                }

                char        kind = types[i].first;
                std::string sig  = types[i].second;
                bool        lli  = (col + 14 < line.size()) && (line[col + 14] & 1);

                auto& so = sig_obs[sig];
                if (kind == 'C') {
                    so.pseudo     = val;
                    so.has_pseudo = true;
                } else if (kind == 'L') {
                    so.phase     = val;
                    so.has_phase = true;
                    so.lli       = lli;
                } else if (kind == 'D') {
                    so.doppler = val;
                } else if (kind == 'S') {
                    so.snr = val;
                }
            }

            for (auto const& kv : sig_obs) {
                if (!kv.second.has_pseudo && !kv.second.has_phase) continue;
                auto signal_id = signal_from_rinex(sys, kv.first);
                if (!signal_id.is_valid()) continue;

                ObsEpoch::Measurement m{};
                m.satellite_id  = sv_id;
                m.signal_id     = signal_id;
                m.pseudorange   = kv.second.pseudo;
                m.carrier_phase = kv.second.phase;
                m.doppler       = kv.second.doppler;
                m.snr           = kv.second.snr;
                m.loss_of_lock  = kv.second.lli;
                epoch.measurements.push_back(m);
            }
        }

        callback(epoch);
        epoch_count++;
    }

    INFOF("parsed %ld epochs from %s", epoch_count, path.c_str());
    return true;
}

}  // namespace rinex
}  // namespace format
