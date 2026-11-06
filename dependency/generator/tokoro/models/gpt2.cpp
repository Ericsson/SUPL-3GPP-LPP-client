// GPT2 (Lagler et al. 2013) pressure, temperature, humidity and VMF1 coefficients from the 5x5
// degree grid file. The grid file format and the interpolation follow the GPT2 model; the
// procedure is the same as in Ginan (Geoscience Australia, Apache-2.0) `trop/tropGPT2.cpp`.
#include "gpt2.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(tokoro, gpt2);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, gpt2)

namespace generator {
namespace tokoro {

static constexpr size_t GPT2_LATITUDE_COUNT  = 36;
static constexpr size_t GPT2_LONGITUDE_COUNT = 72;
static constexpr size_t GPT2_NODE_COUNT      = GPT2_LATITUDE_COUNT * GPT2_LONGITUDE_COUNT;
// columns: lat lon, p(5) T(5) Q(5) dT(5), undulation, hs, ah(5), aw(5)
static constexpr size_t GPT2_COLUMN_COUNT = 34;

bool gpt2_load(Gpt2Grid& grid, std::string const& path) {
    std::ifstream file(path);
    if (!file) {
        ERRORF("failed to open GPT2 grid file: %s", path.c_str());
        return false;
    }

    Gpt2Grid loaded{};
    loaded.nodes.reserve(GPT2_NODE_COUNT);

    std::string line;
    size_t      line_number = 0;
    while (std::getline(file, line)) {
        line_number++;
        if (line.empty() || line.find('%') != std::string::npos) continue;

        double values[GPT2_COLUMN_COUNT];
        size_t count  = 0;
        char*  cursor = &line[0];
        while (count < GPT2_COLUMN_COUNT) {
            char* end = nullptr;
            auto  v   = std::strtod(cursor, &end);
            if (end == cursor) break;
            values[count++] = v;
            cursor          = end;
        }
        if (count == 0) continue;
        if (count < GPT2_COLUMN_COUNT) {
            ERRORF("GPT2 grid file %s:%zu: expected %zu columns, got %zu", path.c_str(),
                   line_number, GPT2_COLUMN_COUNT, count);
            return false;
        }

        // The node order is part of the interpolation, check that it is the expected grid
        auto index        = loaded.nodes.size();
        auto expected_lat = 87.5 - 5.0 * static_cast<double>(index / GPT2_LONGITUDE_COUNT);
        auto expected_lon = 2.5 + 5.0 * static_cast<double>(index % GPT2_LONGITUDE_COUNT);
        // the longitude is either 0..360 or -180..180 in the grid files
        auto lon_difference = std::fmod(values[1] - expected_lon, 360.0);
        if (lon_difference < 0.0) lon_difference += 360.0;
        if (std::fabs(values[0] - expected_lat) > 1e-6 ||
            (lon_difference > 1e-6 && lon_difference < 360.0 - 1e-6)) {
            ERRORF("GPT2 grid file %s:%zu: unexpected grid point (%f, %f), expected (%f, %f)",
                   path.c_str(), line_number, values[0], values[1], expected_lat, expected_lon);
            return false;
        }

        Gpt2Grid::Node node{};
        for (size_t k = 0; k < 5; k++) {
            node.pressure[k]    = values[2 + k];
            node.temperature[k] = values[7 + k];
            node.humidity[k]    = values[12 + k] / 1000.0;  // g/kg -> kg/kg
            node.lapse[k]       = values[17 + k] / 1000.0;  // K/km -> K/m
            node.ah[k]          = values[24 + k] / 1000.0;
            node.aw[k]          = values[29 + k] / 1000.0;
        }
        node.undulation = values[22];
        node.height     = values[23];
        loaded.nodes.push_back(node);
    }

    if (loaded.nodes.size() != GPT2_NODE_COUNT) {
        ERRORF("GPT2 grid file %s: expected %zu grid points, got %zu", path.c_str(),
               GPT2_NODE_COUNT, loaded.nodes.size());
        return false;
    }

    grid = std::move(loaded);
    INFOF("loaded GPT2 grid: %s (%zu grid points)", path.c_str(), grid.nodes.size());
    return true;
}

static double series(double const (&a)[5], double cosfy, double sinfy, double coshy, double sinhy) {
    return a[0] + a[1] * cosfy + a[2] * sinfy + a[3] * coshy + a[4] * sinhy;
}

static double sign_of(double x) {
    if (x > 0.0) return 1.0;
    if (x < 0.0) return -1.0;
    return 0.0;
}

bool gpt2_evaluate(Gpt2Grid const& grid, double mjd, double latitude, double longitude,
                   double height, Gpt2Values& values) NOEXCEPT {
    VSCOPE_FUNCTIONF("%f, %f, %f, %f", mjd, latitude * 180.0 / M_PI, longitude * 180.0 / M_PI,
                     height);

    if (grid.nodes.size() != GPT2_NODE_COUNT) return false;

    constexpr double PI           = 3.1415926535897932384626433;
    constexpr double GRAVITY      = 9.80665;
    constexpr double MOLAR_MASS   = 28.965e-3;  // dry air [kg/mol]
    constexpr double GAS_CONSTANT = 8.3143;
    constexpr double ZERO_CELSIUS = 273.15;

    // Reference epoch is 1 January 2000
    auto mjd1  = mjd - 51544.5;
    auto cosfy = std::cos(mjd1 / 365.25 * 2.0 * PI);
    auto coshy = std::cos(mjd1 / 365.25 * 4.0 * PI);
    auto sinfy = std::sin(mjd1 / 365.25 * 2.0 * PI);
    auto sinhy = std::sin(mjd1 / 365.25 * 4.0 * PI);

    // positive longitude in degrees
    auto plon = (longitude < 0.0 ? longitude + 2.0 * PI : longitude) * 180.0 / PI;
    // polar distance in degrees
    auto pdist = (-latitude + PI / 2.0) * 180.0 / PI;

    // index of the nearest grid point
    auto ipod = static_cast<int>(std::floor((pdist + 5.0) / 5.0));
    auto ilon = static_cast<int>(std::floor((plon + 5.0) / 5.0));

    // normalized differences, can be positive or negative
    auto dpod = (pdist - (ipod * 5 - 2.5)) / 5.0;
    auto dlon = (plon - (ilon * 5 - 2.5)) / 5.0;

    if (ipod == 37) ipod = 36;
    if (ilon == 73) ilon = 1;
    if (ipod < 1 || ipod > 36 || ilon < 1 || ilon > 72) return false;

    auto node_index = [&](int pod, int lon) {
        return static_cast<size_t>((pod - 1) * 72 + lon - 1);
    };

    struct Reduced {
        double pressure;     // [hPa]
        double temperature;  // [C]
        double humidity;     // [kg/kg]
        double lapse;        // [K/m]
        double ah;
        double aw;
        double undulation;
    };

    auto reduce = [&](Gpt2Grid::Node const& node) {
        Reduced r{};
        r.undulation = node.undulation;
        auto hgt     = height - node.undulation;  // orthometric height of the station

        auto t0    = series(node.temperature, cosfy, sinfy, coshy, sinhy);
        auto p0    = series(node.pressure, cosfy, sinfy, coshy, sinhy);
        r.humidity = series(node.humidity, cosfy, sinfy, coshy, sinhy);
        r.lapse    = series(node.lapse, cosfy, sinfy, coshy, sinhy);

        // pressure and temperature at the height of the station
        r.temperature = t0 + r.lapse * (hgt - node.height) - ZERO_CELSIUS;
        auto con      = GRAVITY * MOLAR_MASS / (GAS_CONSTANT * t0 * (1.0 + 0.6077 * r.humidity));
        r.pressure    = p0 * std::exp(-con * (hgt - node.height)) / 100.0;

        r.ah = series(node.ah, cosfy, sinfy, coshy, sinhy);
        r.aw = series(node.aw, cosfy, sinfy, coshy, sinhy);
        return r;
    };

    Reduced result{};
    // Nearest neighbour close to the poles, bilinear interpolation otherwise
    if (pdist <= 2.5 || pdist >= 177.5) {
        result = reduce(grid.nodes[node_index(ipod, ilon)]);
    } else {
        auto ipod1 = ipod + static_cast<int>(sign_of(dpod));
        auto ilon1 = ilon + static_cast<int>(sign_of(dlon));
        if (ilon1 == 73) ilon1 = 1;
        if (ilon1 == 0) ilon1 = 72;

        Reduced r[4];
        r[0] = reduce(grid.nodes[node_index(ipod, ilon)]);
        r[1] = reduce(grid.nodes[node_index(ipod1, ilon)]);
        r[2] = reduce(grid.nodes[node_index(ipod, ilon1)]);
        r[3] = reduce(grid.nodes[node_index(ipod1, ilon1)]);

        auto dnpod1 = std::fabs(dpod);
        auto dnpod2 = 1.0 - dnpod1;
        auto dnlon1 = std::fabs(dlon);
        auto dnlon2 = 1.0 - dnlon1;

        auto interpolate = [&](double Reduced::* member) {
            auto r0 = dnpod2 * r[0].*member + dnpod1 * r[1].*member;
            auto r1 = dnpod2 * r[2].*member + dnpod1 * r[3].*member;
            return dnlon2 * r0 + dnlon1 * r1;
        };

        result.pressure    = interpolate(&Reduced::pressure);
        result.temperature = interpolate(&Reduced::temperature);
        result.humidity    = interpolate(&Reduced::humidity);
        result.ah          = interpolate(&Reduced::ah);
        result.aw          = interpolate(&Reduced::aw);
        result.undulation  = interpolate(&Reduced::undulation);
    }

    values.pressure    = result.pressure;
    values.temperature = result.temperature;
    values.water_vapor_pressure =
        result.humidity * result.pressure / (0.622 + 0.378 * result.humidity);
    values.ah         = result.ah;
    values.aw         = result.aw;
    values.undulation = result.undulation;

    VERBOSEF("pressure:    %+.8f hPa", values.pressure);
    VERBOSEF("temperature: %+.8f C", values.temperature);
    VERBOSEF("e:           %+.8f hPa", values.water_vapor_pressure);
    VERBOSEF("ah:          %+.8f", values.ah);
    VERBOSEF("aw:          %+.8f", values.aw);
    return true;
}

}  // namespace tokoro
}  // namespace generator
