#pragma once
#include <core/core.hpp>

#include <memory>
#include <string>
#include <vector>

namespace generator {
namespace tokoro {

// A-priori zenith delay model. It provides the hydrostatic and wet zenith delay when there is no
// SSR tropospheric correction (see `--tkr-use-tropospheric-model`) and for the comparison output
// of the diagnostics. The SSR zenith delays and the height scaling of them do not depend on it.
enum class TroposphereModel {
    Mops,             // SBAS/MOPS table model (default)
    Saastamoinen,     // Saastamoinen, standard atmosphere with 70% relative humidity
    SaastamoinenGpt,  // Saastamoinen, pressure and temperature from GPT
    Gpt2,             // Saastamoinen, pressure, temperature and humidity from a GPT2 grid file
    Vmf3,             // zenith delays from VMF3 grid files
};

// Mapping function from the zenith delays to the slant direction. It is applied to the SSR zenith
// delays and to the zenith delays of the a-priori model.
enum class MappingFunction {
    Niell,   // Niell mapping function (default)
    Simple,  // 1 / sin(elevation)
    Gmf,     // Global Mapping Function
    Gpt2,    // VMF1 mapping function, coefficients from a GPT2 grid file
    Vmf3,    // VMF3 mapping function, coefficients from VMF3 grid files
};

NODISCARD char const* troposphere_model_name(TroposphereModel model) NOEXCEPT;
NODISCARD char const* mapping_function_name(MappingFunction function) NOEXCEPT;
NODISCARD bool parse_troposphere_model(std::string const& name, TroposphereModel& model) NOEXCEPT;
NODISCARD bool parse_mapping_function(std::string const& name, MappingFunction& function) NOEXCEPT;

NODISCARD std::vector<TroposphereModel> all_troposphere_models();
NODISCARD std::vector<MappingFunction> all_mapping_functions();

// `a|b|c` lists, for help texts and error messages
NODISCARD std::string troposphere_model_names();
NODISCARD std::string mapping_function_names();

// True when the model or mapping function needs data that is loaded at runtime
NODISCARD bool troposphere_model_needs_data(TroposphereModel model) NOEXCEPT;
NODISCARD bool mapping_function_needs_data(MappingFunction function) NOEXCEPT;

struct Gpt2Grid;
struct Vmf3Grid;

// Runtime loaded data for the troposphere models. Nothing is loaded by default.
class TroposphereData {
public:
    TroposphereData();
    ~TroposphereData();

    TroposphereData(TroposphereData const&)            = delete;
    TroposphereData& operator=(TroposphereData const&) = delete;

    // GPT2 5x5 degree grid file (e.g. `gpt2_5.grd`, 2592 grid points)
    NODISCARD bool load_gpt2_grid(std::string const& path);

    // One VMF3 grid file (one epoch, e.g. `VMF3_20260607.H00`); call it once for every file.
    NODISCARD bool load_vmf3_grid(std::string const& path);

    // VMF3 orography file with the same grid as the VMF3 grid files
    NODISCARD bool load_vmf3_orography(std::string const& path);

    NODISCARD bool has_gpt2() const NOEXCEPT;
    // True when at least one grid file and the orography file are loaded
    NODISCARD bool has_vmf3() const NOEXCEPT;

    NODISCARD Gpt2Grid const* gpt2() const NOEXCEPT { return mGpt2.get(); }
    NODISCARD Vmf3Grid const* vmf3() const NOEXCEPT { return mVmf3.get(); }

private:
    std::unique_ptr<Gpt2Grid> mGpt2;
    std::unique_ptr<Vmf3Grid> mVmf3;
};

}  // namespace tokoro
}  // namespace generator
