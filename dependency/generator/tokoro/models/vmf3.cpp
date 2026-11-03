// Vienna Mapping Functions 3 (VMF3), Landskron and Boehm (2018). The grid file reading, the
// interpolation, the height reduction of the zenith delays and the evaluation of the b and c
// coefficients follow the same procedure as Ginan (Geoscience Australia, Apache-2.0)
// `trop/tropVMF3.cpp`.
#include "vmf3.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#include "vienna.hpp"

LOGLET_MODULE2(tokoro, vmf3);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, vmf3)

namespace generator {
namespace tokoro {

// Epochs outside of the loaded epochs by more than this are not used [days]
static constexpr double VMF3_EPOCH_TOLERANCE = 0.25;

static double utc_mjd(int year, int month, int day, int hour, int minute, double second) {
    auto utc = ts::Utc::from_date_time(year, month, day, hour, minute, second);
    return utc.julian_date(0.0) - 2400000.5;
}

static void build_axes(Vmf3Grid& grid) {
    grid.latitudes = grid.row_latitude;
    std::sort(grid.latitudes.begin(), grid.latitudes.end());
    grid.latitudes.erase(std::unique(grid.latitudes.begin(), grid.latitudes.end()),
                         grid.latitudes.end());
    grid.longitudes = grid.row_longitude;
    std::sort(grid.longitudes.begin(), grid.longitudes.end());
    grid.longitudes.erase(std::unique(grid.longitudes.begin(), grid.longitudes.end()),
                          grid.longitudes.end());

    grid.rows.assign(grid.latitudes.size() * grid.longitudes.size(), static_cast<size_t>(-1));
    for (size_t row = 0; row < grid.row_latitude.size(); row++) {
        auto lat_it =
            std::lower_bound(grid.latitudes.begin(), grid.latitudes.end(), grid.row_latitude[row]);
        auto lon_it = std::lower_bound(grid.longitudes.begin(), grid.longitudes.end(),
                                       grid.row_longitude[row]);
        auto lat_i  = static_cast<size_t>(lat_it - grid.latitudes.begin());
        auto lon_i  = static_cast<size_t>(lon_it - grid.longitudes.begin());
        grid.rows[lat_i * grid.longitudes.size() + lon_i] = row;
    }
}

bool vmf3_load_grid(Vmf3Grid& grid, std::string const& path) {
    std::ifstream file(path);
    if (!file) {
        ERRORF("failed to open VMF3 grid file: %s", path.c_str());
        return false;
    }

    auto   have_epoch = false;
    double mjd        = 0.0;

    std::vector<double>          row_latitude;
    std::vector<double>          row_longitude;
    std::vector<Vmf3Grid::Value> values;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        if (line.find('!') != std::string::npos) {
            double year, month, day, hour, minute, second;
            if (std::sscanf(line.c_str(), "! Epoch: %lf %lf %lf %lf %lf %lf", &year, &month, &day,
                            &hour, &minute, &second) == 6) {
                mjd =
                    utc_mjd(static_cast<int>(year), static_cast<int>(month), static_cast<int>(day),
                            static_cast<int>(hour), static_cast<int>(minute), second);
                have_epoch = true;
            }
            continue;
        }

        if (!have_epoch) {
            ERRORF("VMF3 grid file %s: no epoch in the header", path.c_str());
            return false;
        }

        double          lat, lon;
        Vmf3Grid::Value value{};
        if (std::sscanf(line.c_str(), "%lf %lf %lf %lf %lf %lf", &lat, &lon, &value.ah, &value.aw,
                        &value.zhd, &value.zwd) != 6) {
            continue;
        }
        if (lon < 0.0) lon += 360.0;
        row_latitude.push_back(lat);
        row_longitude.push_back(lon);
        values.push_back(value);
    }

    if (values.empty()) {
        ERRORF("VMF3 grid file %s: no grid points", path.c_str());
        return false;
    }

    if (!grid.orography.empty() && grid.orography.size() != row_latitude.size()) {
        ERRORF("VMF3 grid file %s: the grid does not match the orography file", path.c_str());
        return false;
    }

    if (!grid.epochs.empty()) {
        // every file must have the same grid as the first file
        if (row_latitude != grid.row_latitude || row_longitude != grid.row_longitude) {
            ERRORF("VMF3 grid file %s: the grid is not the same as in the previous files",
                   path.c_str());
            return false;
        }
    } else {
        grid.row_latitude  = row_latitude;
        grid.row_longitude = row_longitude;
        build_axes(grid);
    }

    for (auto const& epoch : grid.epochs) {
        if (std::fabs(epoch.mjd - mjd) < 1e-9) {
            WARNF("VMF3 grid file %s: epoch is already loaded, replaced", path.c_str());
        }
    }
    grid.epochs.erase(std::remove_if(grid.epochs.begin(), grid.epochs.end(),
                                     [&](Vmf3Grid::Epoch const& e) {
                                         return std::fabs(e.mjd - mjd) < 1e-9;
                                     }),
                      grid.epochs.end());

    Vmf3Grid::Epoch epoch{};
    epoch.mjd    = mjd;
    epoch.values = std::move(values);
    grid.epochs.push_back(std::move(epoch));
    std::sort(grid.epochs.begin(), grid.epochs.end(),
              [](Vmf3Grid::Epoch const& a, Vmf3Grid::Epoch const& b) {
                  return a.mjd < b.mjd;
              });

    INFOF("loaded VMF3 grid: %s (mjd %.4f, %zu grid points)", path.c_str(), mjd,
          grid.row_latitude.size());
    return true;
}

bool vmf3_load_orography(Vmf3Grid& grid, std::string const& path) {
    std::ifstream file(path);
    if (!file) {
        ERRORF("failed to open VMF3 orography file: %s", path.c_str());
        return false;
    }

    std::vector<double> orography;
    std::string         line;
    while (std::getline(file, line)) {
        double value;
        if (std::sscanf(line.c_str(), "%lf", &value) == 1) {
            orography.push_back(value);
        }
    }

    if (orography.empty()) {
        ERRORF("VMF3 orography file %s: no values", path.c_str());
        return false;
    }
    if (!grid.row_latitude.empty() && grid.row_latitude.size() != orography.size()) {
        ERRORF("VMF3 orography file %s: %zu values, but the grid has %zu points", path.c_str(),
               orography.size(), grid.row_latitude.size());
        return false;
    }

    grid.orography = std::move(orography);
    INFOF("loaded VMF3 orography: %s (%zu values)", path.c_str(), grid.orography.size());
    return true;
}

namespace {
struct Bracket {
    size_t lower;
    size_t upper;
    double weight;  // of the upper one
};
}  // namespace

// `wrap`: the values are periodic with the period 360
static Bracket find_bracket(std::vector<double> const& axis, double value, bool wrap) {
    auto count = axis.size();
    if (count == 1) return {0, 0, 0.0};

    auto it = std::upper_bound(axis.begin(), axis.end(), value);
    if (it == axis.begin()) {
        if (wrap) {
            auto span = axis.front() + 360.0 - axis.back();
            return {count - 1, 0, (value + 360.0 - axis.back()) / span};
        }
        return {0, 0, 0.0};
    }
    if (it == axis.end()) {
        if (wrap) {
            auto span = axis.front() + 360.0 - axis.back();
            return {count - 1, 0, (value - axis.back()) / span};
        }
        return {count - 1, count - 1, 0.0};
    }
    auto upper = static_cast<size_t>(it - axis.begin());
    auto lower = upper - 1;
    return {lower, upper, (value - axis[lower]) / (axis[upper] - axis[lower])};
}

bool vmf3_evaluate(Vmf3Grid const& grid, double mjd, double latitude, double longitude,
                   double height, Vmf3Values& values) NOEXCEPT {
    VSCOPE_FUNCTIONF("%f, %f, %f, %f", mjd, latitude * 180.0 / M_PI, longitude * 180.0 / M_PI,
                     height);

    constexpr double PI = 3.1415926535897932384626433;

    if (grid.epochs.empty() || grid.orography.empty() ||
        grid.orography.size() != grid.row_latitude.size()) {
        return false;
    }
    if (grid.epochs.front().mjd - mjd > VMF3_EPOCH_TOLERANCE ||
        mjd - grid.epochs.back().mjd > VMF3_EPOCH_TOLERANCE) {
        VERBOSEF("epoch %f is outside of the loaded VMF3 epochs [%f, %f]", mjd,
                 grid.epochs.front().mjd, grid.epochs.back().mjd);
        return false;
    }

    auto lat_deg = latitude * 180.0 / PI;
    auto lon_deg = std::fmod(longitude * 180.0 / PI, 360.0);
    if (lon_deg < 0.0) lon_deg += 360.0;

    // bracketing epochs (a time outside the epochs, but within the tolerance, uses the closest)
    Bracket time_bracket{};
    {
        auto it = std::upper_bound(grid.epochs.begin(), grid.epochs.end(), mjd,
                                   [](double t, Vmf3Grid::Epoch const& e) {
                                       return t < e.mjd;
                                   });
        if (it == grid.epochs.begin()) {
            time_bracket = {0, 0, 0.0};
        } else if (it == grid.epochs.end()) {
            time_bracket = {grid.epochs.size() - 1, grid.epochs.size() - 1, 0.0};
        } else {
            auto upper   = static_cast<size_t>(it - grid.epochs.begin());
            auto lower   = upper - 1;
            time_bracket = {lower, upper,
                            (mjd - grid.epochs[lower].mjd) /
                                (grid.epochs[upper].mjd - grid.epochs[lower].mjd)};
        }
    }

    auto lat_bracket = find_bracket(grid.latitudes, lat_deg, false);
    auto lon_bracket = find_bracket(grid.longitudes, lon_deg, true);

    auto lon_count = grid.longitudes.size();

    Vmf3Grid::Value interpolated{};
    double          orography = 0.0;

    auto accumulate = [&](double weight, size_t lat_i, size_t lon_j, Vmf3Grid::Epoch const& epoch,
                          double time_weight) {
        if (weight == 0.0 || time_weight == 0.0) return true;
        auto row = grid.rows[lat_i * lon_count + lon_j];
        if (row == static_cast<size_t>(-1)) return false;
        auto const& v = epoch.values[row];
        auto        w = weight * time_weight;
        interpolated.ah += v.ah * w;
        interpolated.aw += v.aw * w;
        interpolated.zhd += v.zhd * w;
        interpolated.zwd += v.zwd * w;
        orography += grid.orography[row] * w;
        return true;
    };

    for (int t = 0; t < 2; t++) {
        auto const& epoch       = grid.epochs[t == 0 ? time_bracket.lower : time_bracket.upper];
        auto        time_weight = t == 0 ? 1.0 - time_bracket.weight : time_bracket.weight;
        for (int la = 0; la < 2; la++) {
            auto lat_i      = la == 0 ? lat_bracket.lower : lat_bracket.upper;
            auto lat_weight = la == 0 ? 1.0 - lat_bracket.weight : lat_bracket.weight;
            for (int lo = 0; lo < 2; lo++) {
                auto lon_j      = lo == 0 ? lon_bracket.lower : lon_bracket.upper;
                auto lon_weight = lo == 0 ? 1.0 - lon_bracket.weight : lon_bracket.weight;
                if (!accumulate(lat_weight * lon_weight, lat_i, lon_j, epoch, time_weight)) {
                    return false;
                }
            }
        }
    }

    // The grid values are for the height of the grid (orography), reduce them to the station
    // height (Landskron and Boehm 2018, section 5.2).
    constexpr double LAPSE_RATE = 0.0000226;

    auto davis = [&](double h) {
        return 1.0 - 0.00266 * std::cos(2.0 * latitude) - 0.28 * 1e-6 * h;
    };

    auto lapse = std::pow(1.0 - LAPSE_RATE * (height - orography), 5.225);
    values.zhd = lapse * interpolated.zhd * davis(orography) / davis(height);
    values.zwd = interpolated.zwd * std::exp(-(height - orography) / 2000.0);
    values.ah  = interpolated.ah;
    values.aw  = interpolated.aw;

    VERBOSEF("orography: %+.4f", orography);
    VERBOSEF("zhd:       %+.8f", values.zhd);
    VERBOSEF("zwd:       %+.8f", values.zwd);
    VERBOSEF("ah:        %+.8f", values.ah);
    VERBOSEF("aw:        %+.8f", values.aw);
    return true;
}

Vmf3Mapping vmf3_mapping(double ah, double aw, double mjd, double latitude, double longitude,
                         double height, double elevation, bool apply_height_correction) NOEXCEPT {
    VSCOPE_FUNCTIONF("%f, %f, %f, %f, %f, %f, %f", ah, aw, mjd, latitude * 180.0 / M_PI,
                     longitude * 180.0 / M_PI, height, elevation * 180.0 / M_PI);

    constexpr double PI   = 3.1415926535897932384626433;
    constexpr int    NMAX = 12;

    auto lat_deg = latitude * 180.0 / PI;
    auto lon_deg = longitude * 180.0 / PI;

    // day of the year (starting at 1) including the fraction of the day
    auto doy = [&] {
        // mjd 40587 is 1970-01-01
        auto days       = mjd - 40587.0;
        auto day_number = static_cast<int64_t>(std::floor(days));
        auto fraction   = days - static_cast<double>(day_number);
        return ts::Utc::from_day_tod(day_number, fraction * 86400.0).day_of_year();
    }();

    // unit vector
    auto x = std::sin(PI / 2.0 - lat_deg * PI / 180.0) * std::cos(lon_deg * PI / 180.0);
    auto y = std::sin(PI / 2.0 - lat_deg * PI / 180.0) * std::sin(lon_deg * PI / 180.0);
    auto z = std::cos(PI / 2.0 - lat_deg * PI / 180.0);

    double vmat[NMAX + 1][NMAX + 1] = {};
    double wmat[NMAX + 1][NMAX + 1] = {};

    vmat[0][0] = 1.0;
    wmat[0][0] = 0.0;
    vmat[1][0] = z * vmat[0][0];
    wmat[1][0] = 0.0;

    for (int j = 1; j < NMAX; j++) {
        auto n         = j + 1;
        vmat[j + 1][0] = ((2 * n - 1) * z * vmat[j][0] - j * vmat[j - 1][0]) / n;
        wmat[j + 1][0] = 0.0;
    }

    for (int j = 0; j < NMAX; j++) {
        auto m             = j + 1;
        vmat[j + 1][j + 1] = (2 * m - 1) * (x * vmat[j][j] - y * wmat[j][j]);
        wmat[j + 1][j + 1] = (2 * m - 1) * (x * wmat[j][j] + y * vmat[j][j]);

        if (j < NMAX - 1) {
            vmat[j + 2][j + 1] = (2 * m + 1) * z * vmat[j + 1][j + 1];
            wmat[j + 2][j + 1] = (2 * m + 1) * z * wmat[j + 1][j + 1];
        }

        for (int k = j + 2; k < NMAX; k++) {
            vmat[k + 1][j + 1] =
                ((2 * (k + 1) - 1) * z * vmat[k][j + 1] - (k + 1 + j) * vmat[k - 1][j + 1]) /
                (k + 1 - m);
            wmat[k + 1][j + 1] =
                ((2 * (k + 1) - 1) * z * wmat[k][j + 1] - (k + 1 + j) * wmat[k - 1][j + 1]) /
                (k + 1 - m);
        }
    }

    // coefficients bh, bw, ch, cw
    double bhc[5] = {};
    double bwc[5] = {};
    double chc[5] = {};
    double cwc[5] = {};

    using namespace vmf3_coefficients;
    auto p = 0;
    for (int j = 0; j <= NMAX; j++) {
        for (int k = 0; k <= j; k++) {
            for (int q = 0; q < 5; q++) {
                bhc[q] += anm_bh[p][q] * vmat[j][k] + bnm_bh[p][q] * wmat[j][k];
                bwc[q] += anm_bw[p][q] * vmat[j][k] + bnm_bw[p][q] * wmat[j][k];
                chc[q] += anm_ch[p][q] * vmat[j][k] + bnm_ch[p][q] * wmat[j][k];
                cwc[q] += anm_cw[p][q] * vmat[j][k] + bnm_cw[p][q] * wmat[j][k];
            }
            p++;
        }
    }

    // seasonal amplitudes
    auto seasonal = [&](double const(&c)[5]) {
        return c[0] + c[1] * std::cos(doy / 365.25 * 2.0 * PI) +
               c[2] * std::sin(doy / 365.25 * 2.0 * PI) + c[3] * std::cos(doy / 365.25 * 4.0 * PI) +
               c[4] * std::sin(doy / 365.25 * 4.0 * PI);
    };
    auto bh = seasonal(bhc);
    auto bw = seasonal(bwc);
    auto ch = seasonal(chc);
    auto cw = seasonal(cwc);

    Vmf3Mapping result{};
    result.hydrostatic = herring_mapping(elevation, ah, bh, ch);
    result.wet         = herring_mapping(elevation, aw, bw, cw);

    if (apply_height_correction) {
        auto a1 = 2.53e-5;
        auto b1 = 5.49e-3;
        auto c1 = 1.14e-3;
        auto h1 = 1.0 / std::sin(elevation) - herring_mapping(elevation, a1, b1, c1);
        result.hydrostatic += h1 * height / 1000.0;
    }

    VERBOSEF("bh: %+.8f bw: %+.8f ch: %+.8f cw: %+.8f", bh, bw, ch, cw);
    VERBOSEF("hydrostatic: %+.14f", result.hydrostatic);
    VERBOSEF("wet:         %+.14f", result.wet);
    return result;
}

}  // namespace tokoro
}  // namespace generator
