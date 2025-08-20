#include "antex.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

LOGLET_MODULE2(format, antex);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(format, antex)

namespace format {
namespace antex {

CONSTEXPR static double PI         = 3.1415926535897932;
CONSTEXPR static double RAD_TO_DEG = 180.0 / PI;
CONSTEXPR static double MM_TO_M    = 1.0e-3;

static bool is_comment(std::string const& line) {
    // ends with "COMMENT             "
    return line.size() == 80 && line.substr(60, 20) == "COMMENT             ";
}

static bool is_end_of_header(std::string const& line) {
    // ends with "END OF HEADER"
    return line.size() == 80 && line.substr(60, 20) == "END OF HEADER       ";
}

static bool is_start_of_antenna(std::string const& line) {
    // ends with "START OF ANTENNA    "
    return line.size() == 80 && line.substr(60, 20) == "START OF ANTENNA    ";
}

static bool is_end_of_antenna(std::string const& line) {
    // ends with "END OF ANTENNA      "
    return line.size() == 80 && line.substr(60, 20) == "END OF ANTENNA      ";
}

static bool is_type_serial_no(std::string const& line) {
    // ends with "TYPE / SERIAL NO    "
    return line.size() == 80 && line.substr(60, 20) == "TYPE / SERIAL NO    ";
}

static bool is_method_by_date(std::string const& line) {
    // ends with "METH / BY / # / DATE"
    return line.size() == 80 && line.substr(60, 20) == "METH / BY / # / DATE";
}

static bool is_dazi(std::string const& line) {
    // starts with "DAZI"
    return line.size() >= 80 && line.substr(60, 20) == "DAZI                ";
}

static bool is_zen1_zen2_dzen(std::string const& line) {
    // starts with "ZEN1 / ZEN2 / DZEN  "
    return line.size() >= 80 && line.substr(60, 20) == "ZEN1 / ZEN2 / DZEN  ";
}

static bool is_frequency_count(std::string const& line) {
    // starts with "# OF FREQUENCIES    "
    return line.size() >= 80 && line.substr(60, 20) == "# OF FREQUENCIES    ";
}

static bool is_valid_from(std::string const& line) {
    // starts with "VALID FROM"
    return line.size() >= 80 && line.substr(60, 20) == "VALID FROM          ";
}

static bool is_valid_until(std::string const& line) {
    // starts with "VALID UNTIL"
    return line.size() >= 80 && line.substr(60, 20) == "VALID UNTIL         ";
}

static bool is_start_of_frequency(std::string const& line) {
    return line.size() >= 80 && line.substr(60, 20) == "START OF FREQUENCY  ";
}

static bool is_end_of_frequency(std::string const& line) {
    return line.size() >= 80 && line.substr(60, 20) == "END OF FREQUENCY    ";
}

#if 0
static bool is_start_of_frequency_rms(std::string const& line) {
    return line.size() >= 80 && line.substr(60, 20) == "START OF FREQ RMS   ";
}

static bool is_end_of_frequency_rms(std::string const& line) {
    return line.size() >= 80 && line.substr(60, 20) == "END OF FREQ RMS     ";
}
#endif

static bool is_north_east_up(std::string const& line) {
    return line.size() >= 80 && line.substr(60, 20) == "NORTH / EAST / UP   ";
}

static std::string trim(std::string const& str) {
    auto start = str.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    auto end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

static bool parse_float(std::string const& str, double& value) {
    try {
        value = std::stod(str);
        return true;
    } catch (std::exception const&) {
        return false;
    }
}

static bool parse_int(std::string const& str, int64_t& value) {
    try {
        value = std::stoll(str);
        return true;
    } catch (std::exception const&) {
        return false;
    }
}

std::unique_ptr<Antex> Antex::from_string(std::string const& data) {
    FUNCTION_SCOPE();

    auto               result = std::unique_ptr<Antex>(new Antex{});
    std::istringstream stream(data);
    std::string        line;

    // Parse header
    TRACEF("---------------- start of header ----------------");
    while (std::getline(stream, line)) {
        if (is_comment(line)) {
            TRACEF("comment: %s", line.c_str());
            continue;
        }
        if (is_end_of_header(line)) {
            break;
        }
        if (line.size() >= 60) {
            auto label = line.substr(60);
            if (label.find("ANTEX VERSION / SYST") != std::string::npos) {
                // Example parse for version/system
                result->header.version = trim(line.substr(0, 8));
                if (line.size() > 20) result->header.system = line.substr(20, 1);
                VERBOSEF("version: %s, system: %s", result->header.version.c_str(),
                         result->header.system.c_str());
            } else if (label.find("PCV TYPE / REFANT") != std::string::npos) {
                // Example parse for PCV type / reference antenna
                if (line.size() >= 1) result->header.pcv_type = line.substr(0, 1);
                if (line.size() >= 40) result->header.refant_type = trim(line.substr(20, 20));
                if (line.size() >= 60) result->header.refant_serial = trim(line.substr(40, 20));
                VERBOSEF("pcv_type: %s, refant_type: %s, refant_serial: %s",
                         result->header.pcv_type.c_str(), result->header.refant_type.c_str(),
                         result->header.refant_serial.c_str());
            } else {
                TRACEF("unhandled: %s", line.c_str());
            }
        }
    }
    TRACEF("---------------- end of header ----------------");

    // Parse antennas
    while (std::getline(stream, line)) {
        if (is_comment(line)) {
            TRACEF("comment: %s", line.c_str());
            continue;
        }
        if (is_start_of_antenna(line)) {
            auto antenna = std::unique_ptr<Antenna>(new Antenna{});
            TRACEF("---------------- start of antenna ----------------");

            if (!std::getline(stream, line)) {
                ERRORF("failed to read antenna type");
                return nullptr;
            } else if (!is_type_serial_no(line)) {
                ERRORF("expected antenna type and serial number");
                return nullptr;
            }

            auto antenna_type   = trim(line.substr(0, 20));
            auto satellite_code = trim(line.substr(20, 20));
            VERBOSEF("antenna type: %s, satellite code: %s", antenna_type.c_str(),
                     satellite_code.c_str());

            antenna->id = SatelliteId::from_string(satellite_code);
            if (!antenna->id.is_valid()) {
                // Only process known satellite IDs
                while (std::getline(stream, line)) {
                    if (is_end_of_antenna(line)) {
                        break;
                    }
                }

                TRACEF("---------------- end of antenna ----------------");
                continue;
            }

            if (!std::getline(stream, line)) {
                ERRORF("failed to read antenna method");
                return nullptr;
            } else if (!is_method_by_date(line)) {
                ERRORF("expected antenna method and date");
                return nullptr;
            }
            TRACEF("antenna method: %s", line.c_str());

            if (!std::getline(stream, line)) {
                ERRORF("failed to read dazi");
                return nullptr;
            } else if (!is_dazi(line)) {
                ERRORF("expected dazi");
                return nullptr;
            }
            TRACEF("dazi: %s", line.c_str());

            auto dazi_str = trim(line.substr(2, 6));

            if (!std::getline(stream, line)) {
                ERRORF("failed to read zen1, zen2, dzen");
                return nullptr;
            } else if (!is_zen1_zen2_dzen(line)) {
                ERRORF("expected zen1, zen2, dzen");
                return nullptr;
            }

            TRACEF("zen: %s", line.c_str());
            auto zen1_str = trim(line.substr(2, 6));
            auto zen2_str = trim(line.substr(8, 6));
            auto dzen_str = trim(line.substr(14, 6));

            if (!std::getline(stream, line)) {
                ERRORF("failed to read frequency count");
                return nullptr;
            } else if (!is_frequency_count(line)) {
                ERRORF("expected frequency count");
                return nullptr;
            }

            TRACEF("frequency count: %s", line.c_str());
            auto frequency_count_str = trim(line.substr(0, 6));

            if (!parse_float(dazi_str, antenna->dazi)) {
                ERRORF("failed to parse dazi: %s", dazi_str.c_str());
                return nullptr;
            } else if (!parse_float(zen1_str, antenna->zen1)) {
                ERRORF("failed to parse zen1: %s", zen1_str.c_str());
                return nullptr;
            } else if (!parse_float(zen2_str, antenna->zen2)) {
                ERRORF("failed to parse zen2: %s", zen2_str.c_str());
                return nullptr;
            } else if (!parse_float(dzen_str, antenna->dzen)) {
                ERRORF("failed to parse dzen: %s", dzen_str.c_str());
                return nullptr;
            } else if (!parse_int(frequency_count_str, antenna->frequency_count)) {
                ERRORF("failed to parse frequency count: %s", frequency_count_str.c_str());
                return nullptr;
            }

            DEBUGF("antenna: %s", antenna->id.name());
            DEBUGF("  dazi: %f", antenna->dazi);
            DEBUGF("  zen1: %f", antenna->zen1);
            DEBUGF("  zen2: %f", antenna->zen2);
            DEBUGF("  dzen: %f", antenna->dzen);
            DEBUGF("  frequency count: %ld", antenna->frequency_count);

            while (std::getline(stream, line)) {
                TRACEF("line: %s", line.c_str());
                if (is_end_of_antenna(line)) {
                    break;
                } else if (is_valid_from(line)) {
                    auto valid_from_year_str   = trim(line.substr(0, 6));
                    auto valid_from_month_str  = trim(line.substr(6, 6));
                    auto valid_from_day_str    = trim(line.substr(12, 6));
                    auto valid_from_hour_str   = trim(line.substr(18, 6));
                    auto valid_from_minute_str = trim(line.substr(24, 6));
                    auto valid_from_second_str = trim(line.substr(30, 13));

                    int64_t year, month, day, hour, minute;
                    double  second;
                    if (!parse_int(valid_from_year_str, year)) {
                        ERRORF("failed to parse valid from year: %s", valid_from_year_str.c_str());
                        return nullptr;
                    } else if (!parse_int(valid_from_month_str, month)) {
                        ERRORF("failed to parse valid from month: %s",
                               valid_from_month_str.c_str());
                        return nullptr;
                    } else if (!parse_int(valid_from_day_str, day)) {
                        ERRORF("failed to parse valid from day: %s", valid_from_day_str.c_str());
                        return nullptr;
                    } else if (!parse_int(valid_from_hour_str, hour)) {
                        ERRORF("failed to parse valid from hour: %s", valid_from_hour_str.c_str());
                        return nullptr;
                    } else if (!parse_int(valid_from_minute_str, minute)) {
                        ERRORF("failed to parse valid from minute: %s",
                               valid_from_minute_str.c_str());
                        return nullptr;
                    } else if (!parse_float(valid_from_second_str, second)) {
                        ERRORF("failed to parse valid from second: %s",
                               valid_from_second_str.c_str());
                        return nullptr;
                    }

                    VERBOSEF("  valid from: %04" PRId64 " %02" PRId64 " %02" PRId64 " "
                             "%02" PRId64 ":%02" PRId64 ":%06.3f",
                             year, month, day, hour, minute, second);

                    antenna->valid_from_set = true;
                    antenna->valid_from =
                        ts::Gps::from_ymdhms(year, month, day, hour, minute, second);
                    DEBUGF("  valid from: %s",
                           ts::Utc{antenna->valid_from}.rtklib_time_string().c_str());
                } else if (is_valid_until(line)) {
                    auto valid_until_year_str   = trim(line.substr(0, 6));
                    auto valid_until_month_str  = trim(line.substr(6, 6));
                    auto valid_until_day_str    = trim(line.substr(12, 6));
                    auto valid_until_hour_str   = trim(line.substr(18, 6));
                    auto valid_until_minute_str = trim(line.substr(24, 6));
                    auto valid_until_second_str = trim(line.substr(30, 13));

                    int64_t year, month, day, hour, minute;
                    double  second;
                    if (!parse_int(valid_until_year_str, year)) {
                        ERRORF("failed to parse valid until year: %s",
                               valid_until_year_str.c_str());
                        return nullptr;
                    } else if (!parse_int(valid_until_month_str, month)) {
                        ERRORF("failed to parse valid until month: %s",
                               valid_until_month_str.c_str());
                        return nullptr;
                    } else if (!parse_int(valid_until_day_str, day)) {
                        ERRORF("failed to parse valid until day: %s", valid_until_day_str.c_str());
                        return nullptr;
                    } else if (!parse_int(valid_until_hour_str, hour)) {
                        ERRORF("failed to parse valid until hour: %s",
                               valid_until_hour_str.c_str());
                        return nullptr;
                    } else if (!parse_int(valid_until_minute_str, minute)) {
                        ERRORF("failed to parse valid until minute: %s",
                               valid_until_minute_str.c_str());
                        return nullptr;
                    } else if (!parse_float(valid_until_second_str, second)) {
                        ERRORF("failed to parse valid until second: %s",
                               valid_until_second_str.c_str());
                        return nullptr;
                    }

                    antenna->valid_until_set = true;
                    antenna->valid_until =
                        ts::Gps::from_ymdhms(year, month, day, hour, minute, second);
                    DEBUGF("  valid until: %s",
                           ts::Utc{antenna->valid_until}.rtklib_time_string().c_str());
                } else if (is_start_of_frequency(line)) {
                    auto frequency = std::unique_ptr<Frequency>(new Frequency{});
                    TRACEF("---------------- start of frequency ----------------");

                    frequency->dazi = antenna->dazi;
                    frequency->zen1 = antenna->zen1;
                    frequency->zen2 = antenna->zen2;
                    frequency->dzen = antenna->dzen;

                    auto frequency_number_str = trim(line.substr(3, 3));
                    if (frequency_number_str == "G01") {
                        frequency->type = FrequencyType::L1;
                    } else if (frequency_number_str == "G02") {
                        frequency->type = FrequencyType::L2;
                    } else if (frequency_number_str == "G05") {
                        frequency->type = FrequencyType::L5;
                    } else if (frequency_number_str == "R01") {
                        frequency->type = FrequencyType::G1;
                    } else if (frequency_number_str == "R02") {
                        frequency->type = FrequencyType::G2;
                    } else if (frequency_number_str == "E01") {
                        frequency->type = FrequencyType::E1;
                    } else if (frequency_number_str == "E05") {
                        frequency->type = FrequencyType::E5a;
                    } else if (frequency_number_str == "E07") {
                        frequency->type = FrequencyType::E5b;
                    } else if (frequency_number_str == "E08") {
                        frequency->type = FrequencyType::E5;
                    } else if (frequency_number_str == "E06") {
                        frequency->type = FrequencyType::E6;
                    } else if (frequency_number_str == "C01") {
                        frequency->type = FrequencyType::B1;
                    } else if (frequency_number_str == "C02") {
                        frequency->type = FrequencyType::B2;
                    } else if (frequency_number_str == "C06") {
                        frequency->type = FrequencyType::B3;
                    } else if (frequency_number_str == "C05") {
                        frequency->type = FrequencyType::B2a;
                    } else if (frequency_number_str == "C07") {
                        frequency->type = FrequencyType::B2b;
                    } else if (frequency_number_str == "C08") {
                        frequency->type = FrequencyType::B2ab;
                    } else {
                        WARNF("unhandled frequency number: %s", frequency_number_str.c_str());
                        while (std::getline(stream, line)) {
                            if (is_end_of_frequency(line)) {
                                break;
                            }
                        }

                        TRACEF("---------------- end of frequency ----------------");
                        continue;
                    }

                    if (!std::getline(stream, line)) {
                        ERRORF("failed to read north, east, and up");
                        return nullptr;
                    } else if (!is_north_east_up(line)) {
                        ERRORF("expected frequency number");
                        return nullptr;
                    }

                    TRACEF("  NEW: %s", line.c_str());
                    auto north_str = trim(line.substr(0, 10));
                    auto east_str  = trim(line.substr(10, 10));
                    auto up_str    = trim(line.substr(20, 10));

                    if (!std::getline(stream, line)) {
                        ERRORF("failed to read no azimuth");
                        return nullptr;
                    }

                    TRACEF("  NOAZI: %s", line.c_str());
                    auto prefix = line.substr(0, 8);
                    if (prefix != "   NOAZI") {
                        ERRORF("expected no azimuth");
                        return nullptr;
                    }

                    auto element_count =
                        (int64_t)((antenna->zen2 - antenna->zen1) / antenna->dzen) + 1;
                    auto expected_length = 8 + 8 * element_count;
                    TRACEF("  %f - %f = %f / %f, expected %" PRId64, antenna->zen2, antenna->zen1,
                           antenna->zen2 - antenna->zen1, antenna->dzen, element_count);
                    if (line.size() != (size_t)expected_length) {
                        ERRORF("expected no azimuth length %" PRId64 ", got %zu", expected_length,
                               line.size());
                        return nullptr;
                    }

                    for (int64_t i = 0; i < element_count; i++) {
                        auto   no_azimuth_str = trim(line.substr(8 + 8 * i, 8));
                        double no_azimuth;
                        if (!parse_float(no_azimuth_str, no_azimuth)) {
                            ERRORF("failed to parse no azimuth: %s", no_azimuth_str.c_str());
                            return nullptr;
                        }

                        frequency->no_azimuth.push_back(no_azimuth);
                    }

                    if (antenna->dazi > 0.0) {
                        auto dazi_count = (int64_t)(360.0 / antenna->dazi) + 1;
                        for (int64_t i = 0; i < dazi_count; i++) {
                            if (!std::getline(stream, line)) {
                                ERRORF("failed to read azimuth %ld", i);
                                return nullptr;
                            }

                            TRACEF("  AZI: %s", line.c_str());
                            auto azimuth_str = trim(line.substr(0, 8));
                            auto azimuth     = 0.0;
                            if (!parse_float(azimuth_str, azimuth)) {
                                ERRORF("failed to parse azimuth %ld: %s", i, azimuth_str.c_str());
                                return nullptr;
                            }

                            auto expected_length = 8 + 8 * element_count;
                            if (line.size() != (size_t)expected_length) {
                                ERRORF("expected azimuth length %ld, got %zu", expected_length,
                                       line.size());
                                return nullptr;
                            }

                            std::vector<double> values;
                            for (int64_t j = 0; j < element_count; j++) {
                                auto   value_str = trim(line.substr(8 + 8 * j, 8));
                                double value;
                                if (!parse_float(value_str, value)) {
                                    ERRORF("failed to parse value %ld %ld: %s", i, j,
                                           value_str.c_str());
                                    return nullptr;
                                }

                                values.push_back(value);
                            }

                            frequency->azimuths.push_back(std::move(values));
                        }
                    }

                    while (std::getline(stream, line)) {
                        if (is_end_of_frequency(line)) {
                            break;
                        } else {
                            TRACEF("unhandled: %s", line.c_str());
                        }
                    }

                    TRACEF("---------------- end of frequency ----------------");

                    antenna->frequencies[frequency->type] = std::move(frequency);
                } else {
                    TRACEF("unhandled: %s", line.c_str());
                }
            }

            TRACEF("---------------- end of antenna ----------------");
            result->antennas[antenna->id].push_back(std::move(antenna));
        } else {
            TRACEF("unhandled: %s", line.c_str());
        }
    }

    // Sort antennas by the inverse valid from time
    for (auto& [satellite_id, antenna_list] : result->antennas) {
        std::sort(antenna_list.begin(), antenna_list.end(),
                  [](std::unique_ptr<Antenna> const& a, std::unique_ptr<Antenna> const& b) {
                      return a->valid_from > b->valid_from;
                  });
    }

    return result;
}

std::unique_ptr<Antex> Antex::from_file(std::string const& path) {
    FUNCTION_SCOPE();
    std::ifstream file;
    file.open(path);
    if (!file.is_open()) {
        ERRORF("failed to open file %s", path.c_str());
        return nullptr;
    }

    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    VERBOSEF("loaded %zu bytes from %s", data.size(), path.c_str());
    return from_string(data);
}

bool Frequency::phase_variation(double azimuth_rad, double nadir_rad,
                                PhaseVariation& phase_variation) const {
    FUNCTION_SCOPE();

    auto azimuth = azimuth_rad * RAD_TO_DEG;
    if (azimuth < 0.0) azimuth = 0.0;
    if (azimuth >= 360.0) azimuth = 359.9999;

    auto nadir = nadir_rad * RAD_TO_DEG;
    if (nadir < zen1) nadir = zen1;
    if (nadir > zen2) nadir = zen2;

    auto nadir_index  = (nadir - zen1) / dzen;
    auto nadir_index1 = (int)nadir_index;
    auto nadir_index2 = nadir_index1 + 1;
    auto nadir_frac   = nadir_index - nadir_index1;
    VERBOSEF("nad: %d %d %.4f (%.4f) (%7.4f < %7.4f < %7.4f) %.4f", nadir_index1, nadir_index2,
             nadir_frac, nadir_index, zen1, nadir_rad * RAD_TO_DEG, zen2, dzen);
    if (azimuths.empty()) {
        if ((size_t)nadir_index1 >= no_azimuth.size()) {
            VERBOSEF("nad index oob: %zu %d", no_azimuth.size(), nadir_index1);
            return false;
        } else if ((size_t)nadir_index2 >= no_azimuth.size()) {
            VERBOSEF("nad index oob: %zu %d", no_azimuth.size(), nadir_index2);
            return false;
        }

        auto value1 = no_azimuth[nadir_index1];
        auto value2 = no_azimuth[nadir_index2];
        auto value  = value1 + (value2 - value1) * nadir_frac;
        DEBUGF("no azimuth: %.4f %.4f = %.4f", value1, value2, value);
        phase_variation.value = value * MM_TO_M;
        return true;
    } else {
        auto azi        = (azimuth / dazi);
        auto azi_index1 = (int)azi;
        auto azi_index2 = azi_index1 + 1;
        auto azi_frac   = azi - azi_index1;
        VERBOSEF("azi: %d %d %.4f (%.4f) (%7.4f < %7.4f < %7.4f) %.4f", azi_index1, azi_index2,
                 azi_frac, azi, 0.0, azimuth_rad * RAD_TO_DEG, 360.0, dazi);

        if ((size_t)azi_index1 >= azimuths.size()) {
            VERBOSEF("azi index oob: %zu %d", azimuths.size(), azi_index1);
            return false;
        } else if ((size_t)azi_index2 >= azimuths.size()) {
            VERBOSEF("azi index oob: %zu %d", azimuths.size(), azi_index2);
            return false;
        }

        auto& values1 = azimuths[azi_index1];
        auto& values2 = azimuths[azi_index2];
        if ((size_t)nadir_index1 >= values1.size()) {
            VERBOSEF("nad index oob: %zu %d", values1.size(), nadir_index1);
            return false;
        } else if ((size_t)nadir_index2 >= values1.size()) {
            VERBOSEF("nad index oob: %zu %d", values1.size(), nadir_index2);
            return false;
        }

        auto v00   = values1[nadir_index1];
        auto v01   = values1[nadir_index2];
        auto v10   = values2[nadir_index1];
        auto v11   = values2[nadir_index2];
        auto v0    = v00 + (v01 - v00) * nadir_frac;
        auto v1    = v10 + (v11 - v10) * nadir_frac;
        auto value = v0 + (v1 - v0) * azi_frac;

        DEBUGF("%.4f %.4f = %.4f", v00, v01, v0);
        DEBUGF("%.4f %.4f = %.4f", v10, v11, v1);
        DEBUGF("      azimuth = %.4f", value);
        phase_variation.value = value * MM_TO_M;
        return true;
    }
}

bool Antenna::phase_variation(SignalId const& signal_id, double azimuth_rad, double nadir_rad,
                              PhaseVariation& phase_variation) const {
    FUNCTION_SCOPEF("%s %s", id.name(), signal_id.name());

    auto it = frequencies.find(signal_id.frequency_type());
    if (it == frequencies.end()) {
        VERBOSEF("missing frequency for %s", signal_id.name());
        return false;
    }

    return it->second->phase_variation(azimuth_rad, nadir_rad, phase_variation);
}

bool Antex::phase_variation(SatelliteId const& satellite_id, SignalId const& signal_id,
                            ts::Tai const& time, double azimuth_rad, double nadir_rad,
                            PhaseVariation& phase_variation) const {
    FUNCTION_SCOPEF("%s %s %s", satellite_id.name(), signal_id.name(),
                    ts::Utc{time}.rtklib_time_string().c_str());

    auto it = antennas.find(satellite_id);
    if (it == antennas.end()) {
        VERBOSEF("missing antenna for %s", satellite_id.name());
        return false;
    }

    auto& antenna_list = it->second;
    if (antenna_list.empty()) {
        VERBOSEF("empty antenna list for %s", satellite_id.name());
        return false;
    }

    auto gps_time = ts::Gps{time};
    for (auto& antenna : antenna_list) {
        if (antenna->valid_from_set && gps_time < antenna->valid_from) {
            VERBOSEF("%s not valid: %s < %s", satellite_id.name(),
                     ts::Utc{gps_time}.rtklib_time_string().c_str(),
                     ts::Utc{antenna->valid_from}.rtklib_time_string().c_str());
            continue;
        }

        if (antenna->valid_until_set && gps_time > antenna->valid_until) {
            VERBOSEF("%s not valid: %s > %s", satellite_id.name(),
                     ts::Utc{gps_time}.rtklib_time_string().c_str(),
                     ts::Utc{antenna->valid_until}.rtklib_time_string().c_str());
            continue;
        }

        return antenna->phase_variation(signal_id, azimuth_rad, nadir_rad, phase_variation);
    }

    return false;
}

}  // namespace antex
}  // namespace format
