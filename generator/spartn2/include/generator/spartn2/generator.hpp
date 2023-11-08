#pragma once
#include <generator/spartn2/types.hpp>
#include <memory>
#include <vector>

struct LPP_Message;
struct ProvideAssistanceData_r9_IEs;

namespace generator {
namespace spartn {

/// SPARTN message
class Message {};

struct CorrectionPointSet;
struct OcbData;

/// Generates SPARTN messages based on LPP SSR messages.
class Generator {
public:
    /// Constructor.
    Generator();

    /// Destructor.
    ~Generator();

    /// Generate SPARTN messages based on LPP SSR messages.
    /// @param[in] lpp_message The LPP SSR message.
    /// @return The generated SPARTN messages.
    std::vector<Message> generate(const LPP_Message* lpp_message);

private:
    void find_correction_point_set(const ProvideAssistanceData_r9_IEs* message);
    void find_ocb_corrections(const ProvideAssistanceData_r9_IEs* message);

private:
    uint32_t mGenerationIndex;
    std::unique_ptr<CorrectionPointSet> mCorrectionPointSet;
    std::unique_ptr<OcbData> mOcbData;
};

}  // namespace spartn
}  // namespace generator