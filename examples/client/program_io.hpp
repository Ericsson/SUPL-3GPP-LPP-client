#pragma once
#include <io/input.hpp>
#include <io/output.hpp>

#include "input_format.hpp"
#include "output_format.hpp"
#include "stage.hpp"
#include "tag_registry.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

struct OutputInterface {
    OutputFormat                 format;
    std::unique_ptr<io::Output>  initial_interface;
    std::unique_ptr<OutputStage> stage;
    std::vector<std::string>     include_tags;
    std::vector<std::string>     exclude_tags;
    std::vector<std::string>     stages;

    tags::TagMask include_tag_mask;
    tags::TagMask exclude_tag_mask;

    static OutputInterface create(OutputFormat format, std::unique_ptr<io::Output> interface,
                                  std::vector<std::string> include_tags,
                                  std::vector<std::string> exclude_tags,
                                  std::vector<std::string> stages) {
        return {
            format,
            std::move(interface),
            nullptr,
            std::move(include_tags),
            std::move(exclude_tags),
            std::move(stages),
            tags::TagMask(0),
            tags::TagMask(0),
        };
    }

    NODISCARD inline bool ubx_support() const { return (format & OUTPUT_FORMAT_UBX) != 0; }
    NODISCARD inline bool nmea_support() const { return (format & OUTPUT_FORMAT_NMEA) != 0; }
    NODISCARD inline bool rtcm_support() const { return (format & OUTPUT_FORMAT_RTCM) != 0; }
    NODISCARD inline bool ctrl_support() const { return (format & OUTPUT_FORMAT_CTRL) != 0; }
    NODISCARD inline bool lpp_xer_support() const { return (format & OUTPUT_FORMAT_LPP_XER) != 0; }
    NODISCARD inline bool lpp_uper_support() const {
        return (format & OUTPUT_FORMAT_LPP_UPER) != 0;
    }
    NODISCARD inline bool spartn_support() const { return (format & OUTPUT_FORMAT_SPARTN) != 0; }
    NODISCARD inline bool lfr_support() const { return (format & OUTPUT_FORMAT_LFR) != 0; }
    NODISCARD inline bool possib_support() const { return (format & OUTPUT_FORMAT_POSSIB) != 0; }
    NODISCARD inline bool location_support() const {
        return (format & OUTPUT_FORMAT_LOCATION) != 0;
    }
    NODISCARD inline bool raw_support() const { return (format & OUTPUT_FORMAT_RAW) != 0; }
    NODISCARD inline bool test_support() const { return (format & OUTPUT_FORMAT_TEST) != 0; }

    NODISCARD inline bool accept_tag(uint64_t tag) const {
        return tags::TagMask::filter(include_tag_mask, exclude_tag_mask, tags::Tag(tag));
    }
};

struct ProgramOutput {
    std::vector<OutputInterface> outputs;
};

struct InputInterface {
    InputFormat                format;
    bool                       print;
    std::unique_ptr<io::Input> interface;
    std::vector<std::string>   tags;
    std::vector<std::string>   stages;
    bool                       nmea_lf_only;
    bool                       discard_errors;
    bool                       discard_unknowns;
    bool                       exclude_from_shutdown;
};

struct ProgramInput {
    std::vector<InputInterface> inputs;
    bool                        disable_pipe_buffer_optimization = false;
    bool                        shutdown_on_complete             = false;
    std::chrono::milliseconds   shutdown_delay                   = std::chrono::milliseconds(1000);
};

void dump(ProgramInput const& config);
void dump(ProgramOutput const& config);
