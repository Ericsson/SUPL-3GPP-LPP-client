#pragma once
#include <core/core.hpp>

#include <memory>
#include <vector>

#include <generator/rtcm/rtk_data.hpp>

struct LPP_Message;

namespace generator {
namespace rtcm {

/// RTCM message filter.
struct MessageFilter {
    /// Which GNSS systems to generate RTCM messages for.
    struct {
        bool gps     = true;
        bool glonass = true;
        bool galileo = true;
        bool beidou  = true;
    } systems;

    /// Which MSM messages should be considered.
    struct {
        /// If possible generate MSM4 messages.
        bool msm4 = true;
        /// If possible generate MSM5 messages.
        bool msm5 = true;
        /// If possible generate MSM6 messages.
        bool msm6 = true;
        /// If possible generate MSM7 messages.
        bool msm7 = true;

        /// Force generate MSM4 messages, this may not be lossless. NOTE: Set one of the
        /// force msm booleans at a time, having more than one set to true is not supported.
        bool force_msm4 = false;
        /// Force generate MSM5 messages, this may not be lossless. NOTE: Set one of the
        /// force msm booleans at a time, having more than one set to true is not supported.
        bool force_msm5 = false;
        /// Force generate MSM6 messages, this may not be lossless. NOTE: Set one of the
        /// force msm booleans at a time, having more than one set to true is not supported.
        bool force_msm6 = false;
        /// Force generate MSM7 messages, this may not be lossless. NOTE: Set one of the
        /// force msm booleans at a time, having more than one set to true is not supported.
        bool force_msm7 = false;
    } msm;

    /// Which reference station messages should be generated.
    struct {
        /// Generate MT1005 - Reference station. NOTE: mt1006 has priority over mt1005 if
        /// both are enabled.
        bool mt1005 = true;
        /// Generate MT1006 - Reference station (with height).
        bool mt1006 = true;
        /// Generate MT1032 - Physical reference station.
        bool mt1032 = true;
        /// Reference station information will be generated every nth generation or when
        /// directly include in the LPP message.
        uint32_t include_every_nth_generation = 10;
    } reference_station;

    struct {
        /// Generate MT1030 - GPS residuals.
        bool mt1030 = true;
        /// Generate MT1031 - GLONASS residuals.
        bool mt1031 = true;
    } residuals;

    struct {
        /// Generate MT1230 - Glonass bias information.
        bool mt1230 = true;
    } bias;
};

class Message {
public:
    EXPLICIT Message(uint32_t id, std::vector<uint8_t> data) NOEXCEPT : mId(id),
                                                                        mData(std::move(data)) {}

    NODISCARD uint32_t id() const NOEXCEPT { return mId; }
    NODISCARD const std::vector<uint8_t>& data() const NOEXCEPT { return mData; }

private:
    uint32_t             mId;
    std::vector<uint8_t> mData;
};

struct ReferenceStation;
struct PhysicalReferenceStation;

/// Generates RTCM messages based on LPP RTK messages.
class Generator {
public:
    /// Constructor.
    Generator();

    /// Destructor.
    ~Generator();

    /// Generate RTCM messages based on LPP RTK messages.
    /// @param[in] lpp_message The LPP RTK message.
    /// @param[in] filter The filter to apply to the LPP RTK message.
    /// @return The generated RTCM messages.
    NODISCARD std::vector<Message> generate(LPP_Message const*   lpp_message,
                                            MessageFilter const& filter);

    /// Generate RTCM messages that acts as framing for the LPP encoded data. One LPP message may be
    /// split into multiple RTCM messages. The RTCM message id of these message are 355.
    /// @param[in] message_id The message id of the RTCM messages.
    /// @param[in] lpp_data The LPP encoded data.
    /// @param[in] lpp_data_size The size of the LPP encoded data.
    /// @return The generated RTCM messages.
    NODISCARD static std::vector<Message> generate_framing(int message_id, void const* lpp_data,
                                                           size_t lpp_data_size) NOEXCEPT;

private:
    uint32_t                                  mGenerationIndex;
    std::unique_ptr<ReferenceStation>         mReferenceStation;
    std::unique_ptr<PhysicalReferenceStation> mPhysicalReferenceStation;
};

extern Message generate_msm(uint32_t msm, bool last_msm, GenericGnssId gnss,
                            CommonObservationInfo const& common, Observations const& observations);

extern Message generate_1005(ReferenceStation const& reference_station, bool gps_indicator,
                             bool glonass_indicator, bool galileo_indicator);

extern Message generate_1006(ReferenceStation const& reference_station, bool gps_indicator,
                             bool glonass_indicator, bool galileo_indicator);

extern Message generate_1032(ReferenceStation const&         reference_station,
                             PhysicalReferenceStation const& physical_reference_station);

}  // namespace rtcm
}  // namespace generator
