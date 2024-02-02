#pragma once
#include <generator/spartn2/types.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

struct LPP_Message;
struct ProvideAssistanceData_r9_IEs;

namespace generator {
namespace spartn {

/// SPARTN message
class Message {
public:
    SPARTN_EXPLICIT Message(uint8_t message_type, uint8_t message_subtype, uint32_t message_time,
                            std::vector<uint8_t>&& payload);

    /// Message type
    uint8_t message_type() const { return mMessageType; }
    /// Message subtype
    uint8_t message_subtype() const { return mMessageSubtype; }
    /// Message data
    const std::vector<uint8_t>& payload() const { return mPayload; }

    std::vector<uint8_t> build();

private:
    uint8_t              mMessageType;
    uint8_t              mMessageSubtype;
    uint32_t             mMessageTime;
    std::vector<uint8_t> mPayload;
};

struct CorrectionPointSet;
struct CorrectionData;

/// Generates SPARTN messages based on LPP SSR messages.
class Generator {
public:
    /// Constructor.
    Generator();

    /// Destructor.
    ~Generator();

    void set_ura_override(int ura_override) { mUraOverride = ura_override; }

    void set_continuity_indicator(double continuity_indicator) {
        mContinuityIndicator = continuity_indicator;
    }

    void set_ublox_clock_correction(bool ublox_clock_correction) {
        mUBloxClockCorrection = ublox_clock_correction;
    }

    void set_compute_average_zenith_delay(bool compute_average_zenith_delay) {
        mComputeAverageZenithDelay = compute_average_zenith_delay;
    }

    void set_iode_shift(bool iode_shift) { mIodeShift = iode_shift; }

    void set_generate_ocb(bool generate_ocb) { mGenerateOcb = generate_ocb; }
    void set_generate_hpac(bool generate_hpac) { mGenerateHpac = generate_hpac; }
    void set_generate_gad(bool generate_gad) { mGenerateGad = generate_gad; }

    void set_gps_supported(bool gps_supported) { mGpsSupported = gps_supported; }
    void set_glonass_supported(bool glonass_supported) { mGlonassSupported = glonass_supported; }
    void set_galileo_supported(bool galileo_supported) { mGalileoSupported = galileo_supported; }
    void set_beidou_supported(bool beidou_supported) { mBeidouSupported = beidou_supported; }

    /// Generate SPARTN messages based on LPP SSR messages.
    /// @param[in] lpp_message The LPP SSR message.
    /// @return The generated SPARTN messages.
    std::vector<Message> generate(const LPP_Message* lpp_message);

private:
    void find_correction_point_set(const ProvideAssistanceData_r9_IEs* message);
    void find_ocb_corrections(const ProvideAssistanceData_r9_IEs* message);
    void find_hpac_corrections(const ProvideAssistanceData_r9_IEs* message);

    void generate_gad(long iod, uint32_t epoch_time, long set_id);
    void generate_ocb(long iod);
    void generate_hpac(long iod);

    uint16_t next_area_id() {
        auto id     = mNextAreaId;
        mNextAreaId = (mNextAreaId + 1) % 256;
        return id;
    }

private:
    uint32_t mGenerationIndex;
    uint16_t mNextAreaId;

    std::unordered_map<uint16_t, std::unique_ptr<CorrectionPointSet>> mCorrectionPointSets;
    std::unique_ptr<CorrectionData>                                   mCorrectionData;
    std::vector<Message>                                              mMessages;

    int    mUraOverride;          // <0 = no override
    double mContinuityIndicator;  // <0 = no override
    bool   mUBloxClockCorrection;

    // SF055:
    int mIonosphereQualityOverride;  // <0 = no override
    int mIonosphereQualityDefault;

    bool mComputeAverageZenithDelay;
    bool mGroupByEpochTime;
    bool mIodeShift;

    bool mGenerateGad;
    bool mGenerateOcb;
    bool mGenerateHpac;
    bool mGpsSupported;
    bool mGlonassSupported;
    bool mGalileoSupported;
    bool mBeidouSupported;
};

}  // namespace spartn
}  // namespace generator