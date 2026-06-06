#pragma once
#include <io/input.hpp>
#include <io/output.hpp>

#include <client-io/input_format.hpp>
#include <client-io/output_format.hpp>
#include <client-io/stage.hpp>
#include <client-io/tag_registry.hpp>
#include <client-io/types.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

struct OutputInterface {
    OutputEntry                  entry;
    std::unique_ptr<io::Output>  initial_interface;
    std::unique_ptr<OutputStage> stage;
    tags::TagMask                include_tag_mask;
    tags::TagMask                exclude_tag_mask;

    NODISCARD inline OutputFormat format() const { return entry.format; }

    NODISCARD inline bool ubx_support() const { return (entry.format & OUTPUT_FORMAT_UBX) != 0; }
    NODISCARD inline bool nmea_support() const { return (entry.format & OUTPUT_FORMAT_NMEA) != 0; }
    NODISCARD inline bool rtcm_support() const { return (entry.format & OUTPUT_FORMAT_RTCM) != 0; }
    NODISCARD inline bool ctrl_support() const { return (entry.format & OUTPUT_FORMAT_CTRL) != 0; }
    NODISCARD inline bool lpp_xer_support() const {
        return (entry.format & OUTPUT_FORMAT_LPP_XER) != 0;
    }
    NODISCARD inline bool lpp_uper_support() const {
        return (entry.format & OUTPUT_FORMAT_LPP_UPER) != 0;
    }
    NODISCARD inline bool spartn_support() const {
        return (entry.format & OUTPUT_FORMAT_SPARTN) != 0;
    }
    NODISCARD inline bool lfr_support() const { return (entry.format & OUTPUT_FORMAT_LFR) != 0; }
    NODISCARD inline bool possib_support() const {
        return (entry.format & OUTPUT_FORMAT_POSSIB) != 0;
    }
    NODISCARD inline bool location_support() const {
        return (entry.format & OUTPUT_FORMAT_LOCATION) != 0;
    }
    NODISCARD inline bool raw_support() const { return (entry.format & OUTPUT_FORMAT_RAW) != 0; }
    NODISCARD inline bool test_support() const { return (entry.format & OUTPUT_FORMAT_TEST) != 0; }

    NODISCARD inline bool accept_tag(uint64_t tag) const {
        return tags::TagMask::filter(include_tag_mask, exclude_tag_mask, tags::Tag(tag));
    }

    NODISCARD inline std::string reject_reason(uint64_t tag) const {
        if (include_tag_mask.value != 0 && !(include_tag_mask.value & tag))
            return "not in itags=" + tags::to_string(include_tag_mask);
        if (exclude_tag_mask.value & tag) return "in otags=" + tags::to_string(exclude_tag_mask);
        return "unknown";
    }

    NODISCARD inline std::string tag_name(uint64_t tag) const {
        auto name = tags::to_string(tags::TagMask(tag));
        return name.empty() ? "untagged" : name;
    }
};

struct ProgramOutput {
    std::vector<OutputInterface> outputs;
};

struct InputInterface {
    InputEntry                 entry;
    std::unique_ptr<io::Input> interface;
    tags::TagMask              tag_mask;

    NODISCARD inline InputFormat format() const { return entry.format; }
    NODISCARD inline bool        print() const { return entry.print; }
    NODISCARD inline bool        nmea_lf_only() const { return entry.nmea_lf_only; }
    NODISCARD inline bool        discard_errors() const { return entry.discard_errors; }
    NODISCARD inline bool        discard_unknowns() const { return entry.discard_unknowns; }
    NODISCARD inline bool exclude_from_shutdown() const { return entry.exclude_from_shutdown; }
};

struct ProgramInput {
    std::vector<InputInterface> inputs;
    bool                        disable_pipe_buffer_optimization = false;
    bool                        shutdown_on_complete             = false;
    std::chrono::milliseconds   shutdown_delay                   = std::chrono::milliseconds(1000);
};

void dump(ProgramInput const& config);
void dump(ProgramOutput const& config);
