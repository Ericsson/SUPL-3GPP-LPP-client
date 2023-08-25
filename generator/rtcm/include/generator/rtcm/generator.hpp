#pragma once
#include <generator/rtcm/types.hpp>
#include <vector>
#include <memory>

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
    RTCM_EXPLICIT Message(uint32_t id, std::vector<uint8_t> data) RTCM_NOEXCEPT
        : mId(id),
          mData(std::move(data)) {}

    RTCM_NODISCARD uint32_t id() const RTCM_NOEXCEPT { return mId; }
    RTCM_NODISCARD const std::vector<uint8_t>& data() const RTCM_NOEXCEPT { return mData; }

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
    std::vector<Message> generate(const LPP_Message* lpp_message, const MessageFilter& filter);

private:
    uint32_t mGenerationIndex;
    std::unique_ptr<ReferenceStation> mReferenceStation;
    std::unique_ptr<PhysicalReferenceStation> mPhysicalReferenceStation;
};

}  // namespace rtcm
}  // namespace generator
