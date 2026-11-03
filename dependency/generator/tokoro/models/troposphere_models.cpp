#include "troposphere_models.hpp"

#include <cmath>
#include <cstring>

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#include "constant.hpp"
#include "gmf.hpp"
#include "gpt.hpp"
#include "gpt2.hpp"
#include "helper.hpp"
#include "mops.hpp"
#include "saastamoinen.hpp"
#include "vienna.hpp"
#include "vmf3.hpp"

LOGLET_MODULE2(tokoro, troposphere);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, troposphere)

namespace generator {
namespace tokoro {

// Relative humidity of the models that do not have humidity data
static constexpr double DEFAULT_RELATIVE_HUMIDITY = 0.7;

namespace {
struct ModelName {
    TroposphereModel model;
    char const*      name;
};
struct MappingName {
    MappingFunction function;
    char const*     name;
};
}  // namespace

static ModelName const MODEL_NAMES[] = {
    {TroposphereModel::Mops, "mops"},
    {TroposphereModel::Saastamoinen, "saastamoinen"},
    {TroposphereModel::SaastamoinenGpt, "saastamoinen-gpt"},
    {TroposphereModel::Gpt2, "gpt2"},
    {TroposphereModel::Vmf3, "vmf3"},
};

static MappingName const MAPPING_NAMES[] = {
    {MappingFunction::Niell, "niell"}, {MappingFunction::Simple, "simple"},
    {MappingFunction::Gmf, "gmf"},     {MappingFunction::Gpt2, "gpt2"},
    {MappingFunction::Vmf3, "vmf3"},
};

namespace {
struct GradientSourceName {
    GradientSource source;
    char const*    name;
};
struct GradientMappingName {
    GradientMapping mapping;
    char const*     name;
};
}  // namespace

static GradientSourceName const GRADIENT_SOURCE_NAMES[] = {
    {GradientSource::None, "none"},
    {GradientSource::SsrGrid, "ssr-grid"},
    {GradientSource::Constant, "constant"},
};

static GradientMappingName const GRADIENT_MAPPING_NAMES[] = {
    {GradientMapping::ChenHerring, "chen-herring"},
    {GradientMapping::MacMillan, "macmillan"},
};

char const* gradient_source_name(GradientSource source) NOEXCEPT {
    for (auto const& entry : GRADIENT_SOURCE_NAMES) {
        if (entry.source == source) return entry.name;
    }
    return "unknown";
}

char const* gradient_mapping_name(GradientMapping mapping) NOEXCEPT {
    for (auto const& entry : GRADIENT_MAPPING_NAMES) {
        if (entry.mapping == mapping) return entry.name;
    }
    return "unknown";
}

bool parse_gradient_source(std::string const& name, GradientSource& source) NOEXCEPT {
    for (auto const& entry : GRADIENT_SOURCE_NAMES) {
        if (name == entry.name) {
            source = entry.source;
            return true;
        }
    }
    return false;
}

bool parse_gradient_mapping(std::string const& name, GradientMapping& mapping) NOEXCEPT {
    for (auto const& entry : GRADIENT_MAPPING_NAMES) {
        if (name == entry.name) {
            mapping = entry.mapping;
            return true;
        }
    }
    return false;
}

std::vector<GradientSource> all_gradient_sources() {
    std::vector<GradientSource> sources;
    for (auto const& entry : GRADIENT_SOURCE_NAMES) {
        if (entry.source != GradientSource::None) sources.push_back(entry.source);
    }
    return sources;
}

std::vector<GradientMapping> all_gradient_mappings() {
    std::vector<GradientMapping> mappings;
    for (auto const& entry : GRADIENT_MAPPING_NAMES) {
        mappings.push_back(entry.mapping);
    }
    return mappings;
}

std::string gradient_source_names() {
    std::string names;
    for (auto const& entry : GRADIENT_SOURCE_NAMES) {
        if (!names.empty()) names += '|';
        names += entry.name;
    }
    return names;
}

std::string gradient_mapping_names() {
    std::string names;
    for (auto const& entry : GRADIENT_MAPPING_NAMES) {
        if (!names.empty()) names += '|';
        names += entry.name;
    }
    return names;
}

char const* troposphere_model_name(TroposphereModel model) NOEXCEPT {
    for (auto const& entry : MODEL_NAMES) {
        if (entry.model == model) return entry.name;
    }
    return "unknown";
}

char const* mapping_function_name(MappingFunction function) NOEXCEPT {
    for (auto const& entry : MAPPING_NAMES) {
        if (entry.function == function) return entry.name;
    }
    return "unknown";
}

bool parse_troposphere_model(std::string const& name, TroposphereModel& model) NOEXCEPT {
    for (auto const& entry : MODEL_NAMES) {
        if (name == entry.name) {
            model = entry.model;
            return true;
        }
    }
    return false;
}

bool parse_mapping_function(std::string const& name, MappingFunction& function) NOEXCEPT {
    for (auto const& entry : MAPPING_NAMES) {
        if (name == entry.name) {
            function = entry.function;
            return true;
        }
    }
    return false;
}

std::vector<TroposphereModel> all_troposphere_models() {
    std::vector<TroposphereModel> models;
    for (auto const& entry : MODEL_NAMES) {
        models.push_back(entry.model);
    }
    return models;
}

std::vector<MappingFunction> all_mapping_functions() {
    std::vector<MappingFunction> functions;
    for (auto const& entry : MAPPING_NAMES) {
        functions.push_back(entry.function);
    }
    return functions;
}

std::string troposphere_model_names() {
    std::string names;
    for (auto const& entry : MODEL_NAMES) {
        if (!names.empty()) names += '|';
        names += entry.name;
    }
    return names;
}

std::string mapping_function_names() {
    std::string names;
    for (auto const& entry : MAPPING_NAMES) {
        if (!names.empty()) names += '|';
        names += entry.name;
    }
    return names;
}

bool troposphere_model_needs_data(TroposphereModel model) NOEXCEPT {
    return model == TroposphereModel::Gpt2 || model == TroposphereModel::Vmf3;
}

bool mapping_function_needs_data(MappingFunction function) NOEXCEPT {
    return function == MappingFunction::Gpt2 || function == MappingFunction::Vmf3;
}

TroposphereData::TroposphereData()  = default;
TroposphereData::~TroposphereData() = default;

bool TroposphereData::load_gpt2_grid(std::string const& path) {
    auto grid = std::unique_ptr<Gpt2Grid>(new Gpt2Grid());
    if (!gpt2_load(*grid, path)) return false;
    mGpt2 = std::move(grid);
    return true;
}

bool TroposphereData::load_vmf3_grid(std::string const& path) {
    if (!mVmf3) mVmf3 = std::unique_ptr<Vmf3Grid>(new Vmf3Grid());
    return vmf3_load_grid(*mVmf3, path);
}

bool TroposphereData::load_vmf3_orography(std::string const& path) {
    if (!mVmf3) mVmf3 = std::unique_ptr<Vmf3Grid>(new Vmf3Grid());
    return vmf3_load_orography(*mVmf3, path);
}

bool TroposphereData::has_gpt2() const NOEXCEPT {
    return mGpt2 && !mGpt2->nodes.empty();
}

bool TroposphereData::has_vmf3() const NOEXCEPT {
    return mVmf3 && !mVmf3->epochs.empty() && !mVmf3->orography.empty();
}

double troposphere_mjd(ts::Tai const& time) NOEXCEPT {
    return ts::Utc{time}.julian_date(0.0) - 2400000.5;
}

bool height_scaling(TroposphereSite const& site, double& hydrostatic, double& wet) NOEXCEPT {
    HydrostaticAndWetDelay alt_0{};
    HydrostaticAndWetDelay alt_eh{};
    auto mops_0 = mops_tropospheric_delay(site.time, site.llh.x, 0.0, site.geoid_height, alt_0);
    auto mops_eh =
        mops_tropospheric_delay(site.time, site.llh.x, site.llh.z, site.geoid_height, alt_eh);
    if (!mops_0 || !mops_eh) return false;

    hydrostatic = alt_eh.hydrostatic / alt_0.hydrostatic;
    wet         = alt_eh.wet / alt_0.wet;
    return true;
}

// Chen and Herring (1997), as given in IERS Conventions 2010 section 9.2
static constexpr double CHEN_HERRING_C = 0.0032;

// Scale heights of the hydrostatic (1 / (5.225 * 2.26e-5), the VMF3 height reduction) and the wet
// (the VMF3 height reduction) delays
static constexpr double HYDROSTATIC_SCALE_HEIGHT = 8470.0;
static constexpr double WET_SCALE_HEIGHT         = 2000.0;

double gradient_mapping_value(GradientMapping mapping, double elevation,
                              double hydrostatic_mapping) NOEXCEPT {
    if (elevation <= 0.0) return 0.0;
    auto sine   = std::sin(elevation);
    auto cosine = std::cos(elevation);
    if (cosine < 1e-12) return 0.0;  // zenith

    switch (mapping) {
    case GradientMapping::ChenHerring: return 1.0 / (sine * (sine / cosine) + CHEN_HERRING_C);
    case GradientMapping::MacMillan:
        if (sine < 1e-6) sine = 1e-6;
        return hydrostatic_mapping * (cosine / sine);
    }
    return 0.0;
}

double gradient_delay(double gradient_mapping, double azimuth, double north, double east) NOEXCEPT {
    return gradient_mapping * (north * std::cos(azimuth) + east * std::sin(azimuth));
}

bool ssr_grid_gradient(TroposphericCorrection const& ssr, double hydrostatic_scale,
                       double wet_scale, double& north, double& east) NOEXCEPT {
    if (!ssr.has_gradient) return false;
    north = HYDROSTATIC_SCALE_HEIGHT * hydrostatic_scale * ssr.dry_north +
            WET_SCALE_HEIGHT * wet_scale * ssr.wet_north;
    east = HYDROSTATIC_SCALE_HEIGHT * hydrostatic_scale * ssr.dry_east +
           WET_SCALE_HEIGHT * wet_scale * ssr.wet_east;
    return true;
}

bool zenith_delays(TroposphereModel model, TroposphereData const* data, TroposphereSite const& site,
                   ZenithDelays& delays) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %f, %f, %f", troposphere_model_name(model),
                     site.llh.x * constant::RAD2DEG, site.llh.y * constant::RAD2DEG, site.llh.z);

    auto latitude  = site.llh.x;
    auto longitude = site.llh.y;
    auto height    = site.llh.z;

    switch (model) {
    case TroposphereModel::Mops: {
        HydrostaticAndWetDelay delay{};
        if (!mops_tropospheric_delay(site.time, latitude, height, site.geoid_height, delay)) {
            return false;
        }
        delays = {delay.hydrostatic, delay.wet};
        return true;
    }
    case TroposphereModel::Saastamoinen: {
        SaastamoinenDelay delay{};
        if (!saastamoinen_standard_atmosphere(latitude, height, DEFAULT_RELATIVE_HUMIDITY, delay)) {
            return false;
        }
        delays = {delay.hydrostatic, delay.wet};
        return true;
    }
    case TroposphereModel::SaastamoinenGpt: {
        if (height < -1000.0 || height > 20000.0) return false;
        auto values      = gpt(troposphere_mjd(site.time), latitude, longitude, height);
        auto temperature = values.temperature + 273.15;
        auto vapor       = water_vapor_pressure(temperature, DEFAULT_RELATIVE_HUMIDITY);
        auto delay = saastamoinen_delay(latitude, height, values.pressure, temperature, vapor);
        delays     = {delay.hydrostatic, delay.wet};
        return true;
    }
    case TroposphereModel::Gpt2: {
        if (data == nullptr || !data->has_gpt2()) return false;
        if (height < -1000.0 || height > 20000.0) return false;
        Gpt2Values values{};
        if (!gpt2_evaluate(*data->gpt2(), troposphere_mjd(site.time), latitude, longitude, height,
                           values)) {
            return false;
        }
        auto delay = saastamoinen_delay(latitude, height, values.pressure,
                                        values.temperature + 273.15, values.water_vapor_pressure);
        delays     = {delay.hydrostatic, delay.wet};
        return true;
    }
    case TroposphereModel::Vmf3: {
        if (data == nullptr || !data->has_vmf3()) return false;
        Vmf3Values values{};
        if (!vmf3_evaluate(*data->vmf3(), troposphere_mjd(site.time), latitude, longitude, height,
                           values)) {
            return false;
        }
        delays = {values.zhd, values.zwd};
        return true;
    }
    }
    return false;
}

bool mapping_values(MappingFunction function, TroposphereData const* data,
                    TroposphereSite const& site, double elevation, bool apply_height_correction,
                    MappingValues& values) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %f, %f, %f, %f", mapping_function_name(function),
                     site.llh.x * constant::RAD2DEG, site.llh.y * constant::RAD2DEG, site.llh.z,
                     elevation * constant::RAD2DEG);

    if (elevation < 0.0) {
        values = {0.0, 0.0};
        return true;
    }

    auto latitude  = site.llh.x;
    auto longitude = site.llh.y;
    auto height    = site.llh.z;

    switch (function) {
    case MappingFunction::Niell: {
        auto mapping =
            hydrostatic_mapping_function(site.time, site.llh, elevation, apply_height_correction);
        values = {mapping.hydrostatic, mapping.wet};
        return true;
    }
    case MappingFunction::Simple: {
        // avoid the singularity at the horizon
        auto sine = std::sin(elevation);
        if (sine < 1e-6) sine = 1e-6;
        values = {1.0 / sine, 1.0 / sine};
        return true;
    }
    case MappingFunction::Gmf: {
        auto mapping =
            global_mapping_function(troposphere_mjd(site.time), latitude, longitude, height,
                                    constant::PI / 2.0 - elevation, apply_height_correction);
        values = {mapping.hydrostatic, mapping.wet};
        return true;
    }
    case MappingFunction::Gpt2: {
        if (data == nullptr || !data->has_gpt2()) return false;
        auto       mjd = troposphere_mjd(site.time);
        Gpt2Values gpt2_values{};
        if (!gpt2_evaluate(*data->gpt2(), mjd, latitude, longitude, height, gpt2_values)) {
            return false;
        }
        auto mapping = vmf1_mapping(gpt2_values.ah, gpt2_values.aw, mjd, latitude, height,
                                    elevation, apply_height_correction);
        values       = {mapping.hydrostatic, mapping.wet};
        return true;
    }
    case MappingFunction::Vmf3: {
        if (data == nullptr || !data->has_vmf3()) return false;
        auto       mjd = troposphere_mjd(site.time);
        Vmf3Values vmf3_values{};
        if (!vmf3_evaluate(*data->vmf3(), mjd, latitude, longitude, height, vmf3_values)) {
            return false;
        }
        auto mapping = vmf3_mapping(vmf3_values.ah, vmf3_values.aw, mjd, latitude, longitude,
                                    height, elevation, apply_height_correction);
        values       = {mapping.hydrostatic, mapping.wet};
        return true;
    }
    }
    return false;
}

}  // namespace tokoro
}  // namespace generator
