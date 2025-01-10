#if defined(INCLUDE_GENERATOR_SPARTN)
#include "lpp2spartn.hpp"

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "p/l2s"

Lpp2Spartn::Lpp2Spartn(OutputConfig const& output, Lpp2SpartnConfig const& config)
    : mOutput(output), mConfig(config) {
    VSCOPE_FUNCTION();
    mGenerator = std::unique_ptr<generator::spartn::Generator>(new generator::spartn::Generator{});

    mGenerator->set_ura_override(mConfig.sf024_override);
    mGenerator->set_ura_default(mConfig.sf024_default);
    mGenerator->set_ublox_clock_correction(mConfig.ublox_clock_correction);
    mGenerator->set_continuity_indicator(320.0);
    mGenerator->set_compute_average_zenith_delay(mConfig.average_zenith_delay);

    if (mConfig.sf055_override >= 0) mGenerator->set_sf055_override(mConfig.sf055_override);
    if (mConfig.sf055_default >= 0) mGenerator->set_sf055_default(mConfig.sf055_default);
    if (mConfig.sf042_override >= 0) mGenerator->set_sf042_override(mConfig.sf042_override);
    if (mConfig.sf042_default >= 0) mGenerator->set_sf042_default(mConfig.sf042_default);

    mGenerator->set_increasing_siou(mConfig.increasing_siou);
    mGenerator->set_filter_by_residuals(mConfig.filter_by_residuals);
    mGenerator->set_filter_by_ocb(mConfig.filter_by_ocb);
    mGenerator->set_ignore_l2l(mConfig.ignore_l2l);
    mGenerator->set_stec_invalid_to_zero(mConfig.stec_invalid_to_zero);

    mGenerator->set_code_bias_translate(mConfig.code_bias_translate);
    mGenerator->set_code_bias_correction_shift(mConfig.code_bias_correction_shift);
    mGenerator->set_phase_bias_translate(mConfig.phase_bias_translate);
    mGenerator->set_phase_bias_correction_shift(mConfig.phase_bias_correction_shift);

    mGenerator->set_hydrostatic_in_zenith(mConfig.hydrostatic_in_zenith);
    mGenerator->set_stec_method(mConfig.stec_method);
    mGenerator->set_stec_transform(mConfig.stec_transform);
    mGenerator->set_sign_flip_c00(mConfig.sign_flip_c00);
    mGenerator->set_sign_flip_c01(mConfig.sign_flip_c01);
    mGenerator->set_sign_flip_c10(mConfig.sign_flip_c10);
    mGenerator->set_sign_flip_c11(mConfig.sign_flip_c11);
    mGenerator->set_sign_flip_stec_residuals(mConfig.sign_flip_stec_residuals);

    mGenerator->set_gps_supported(mConfig.generate_gps);
    mGenerator->set_glonass_supported(mConfig.generate_glonass);
    mGenerator->set_galileo_supported(mConfig.generate_galileo);
    mGenerator->set_beidou_supported(mConfig.generate_beidou);

    mGenerator->set_flip_grid_bitmask(mConfig.flip_grid_bitmask);
    mGenerator->set_flip_orbit_correction(mConfig.flip_orbit_correction);

    mGenerator->set_generate_gad(mConfig.generate_gad);
    mGenerator->set_generate_ocb(mConfig.generate_ocb);
    mGenerator->set_generate_hpac(mConfig.generate_hpac);
}

Lpp2Spartn::~Lpp2Spartn() {
    VSCOPE_FUNCTION();
}

void Lpp2Spartn::inspect(streamline::System&, DataType const& message) {
    VSCOPE_FUNCTION();
    auto messages = mGenerator->generate(message.get());
    if (messages.empty()) {
        WARNF("no SPARTN messages generated, check that you're using `--ad-type ssr`");
    } else {
        INFOF("generated %zu SPARTN messages", messages.size());
        LOGLET_DINDENT_SCOPE();
        for (auto& msg : messages) {
            auto data = msg.build();
            DEBUGF("message: %02X %02X: %zu bytes", msg.message_type(), msg.message_subtype(),
                   data.size());

            // TODO(ewasjon): These message should be passed back into the system
            for (auto const& output : mOutput.outputs) {
                if (!output.spartn_support()) continue;
                if (output.print) {
                    XINFOF(OUTPUT_PRINT_MODULE, "spartn: %02X %02X (%zd bytes)", msg.message_type(),
                           msg.message_subtype(), data.size());
                }
                output.interface->write(data.data(), data.size());
            }
        }
    }
}

#endif
