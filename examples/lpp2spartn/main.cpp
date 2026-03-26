#include <args.hxx>
#include <generator/spartn2/generator.hpp>
#include <loglet/loglet.hpp>

#include <LPP-Message.h>
#include <asn_application.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>

LOGLET_MODULE(main);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(main)

static std::vector<uint8_t> read_file(std::string const& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        fprintf(stderr, "error: cannot open file: %s\n", path.c_str());
        exit(1);
    }
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

static std::vector<uint8_t> read_stdin() {
    std::vector<uint8_t> buffer;
    int                  c;
    while ((c = fgetc(stdin)) != EOF) {
        buffer.push_back(static_cast<uint8_t>(c));
    }
    return buffer;
}

static LPP_Message* decode_lpp(uint8_t const* data, size_t size, size_t* consumed) {
    asn_codec_ctx_t ctx{};
    ctx.max_stack_size = 1024 * 1024 * 4;

    LPP_Message* message = nullptr;
    auto         result  = uper_decode_complete(&ctx, &asn_DEF_LPP_Message,
                                                reinterpret_cast<void**>(&message), data, size);
    if (consumed) *consumed = result.consumed;

    if (result.code != RC_OK) {
        ASN_STRUCT_FREE(asn_DEF_LPP_Message, message);
        return nullptr;
    }
    return message;
}

int main(int argc, char** argv) {
    args::ArgumentParser parser("LPP UPER to SPARTN converter");
    args::HelpFlag       help(parser, "help", "Show help", {'h', "help"});

    args::Group                  input_group(parser, "Input:");
    args::ValueFlag<std::string> input_file(input_group, "file",
                                            "Input LPP UPER file (default: stdin)", {'i', "input"});
    args::ValueFlag<size_t> limit(input_group, "count", "Limit number of LPP messages to process",
                                  {'n', "limit"});

    args::Group                  output_group(parser, "Output:");
    args::ValueFlag<std::string> output_file(output_group, "file", "Output file (default: stdout)",
                                             {'o', "output"});
    args::Flag hex_output(output_group, "hex", "Output as hex instead of binary", {"hex"});

    args::Group gnss_group(parser, "GNSS:");
    args::Flag  gps(gnss_group, "gps", "Enable GPS", {"gps"});
    args::Flag  glonass(gnss_group, "glonass", "Enable GLONASS", {"glonass"});
    args::Flag  galileo(gnss_group, "galileo", "Enable Galileo", {"galileo"});
    args::Flag  beidou(gnss_group, "beidou", "Enable BeiDou", {"beidou"});
    args::Flag  qzss(gnss_group, "qzss", "Enable QZSS", {"qzss"});
    args::Flag  navic(gnss_group, "navic", "Enable NavIC", {"navic"});

    args::Group msg_group(parser, "Messages:");
    args::Flag  no_ocb(msg_group, "no-ocb", "Disable OCB generation", {"no-ocb"});
    args::Flag  no_hpac(msg_group, "no-hpac", "Disable HPAC generation", {"no-hpac"});
    args::Flag  no_gad(msg_group, "no-gad", "Disable GAD generation", {"no-gad"});
    args::Flag  no_dnu(msg_group, "no-dnu", "Disable Do-Not-Use satellite flag", {"no-dnu"});

    args::Group                  transport_group(parser, "Transport:");
    args::ValueFlag<std::string> crc_type(transport_group, "crc8|crc16|crc24q",
                                          "CRC type (default: crc16)", {"crc-type"});
    args::ValueFlag<int> solution_id(transport_group, "0-127", "Solution ID / TF010 (default: 0)",
                                     {"solution-id"});
    args::ValueFlag<int> solution_processor_id(transport_group, "0-15",
                                               "Solution Processor ID / TF011 (default: 0)",
                                               {"solution-processor-id"});

    args::Group log_group(parser, "Logging:");
    args::Flag  trace(log_group, "trace", "Trace logging", {'v', "trace"});
    args::Flag  verbose(log_group, "verbose", "Verbose logging", {'v', "verbose"});
    args::Flag  debug(log_group, "debug", "Debug logging", {'d', "debug"});

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help const&) {
        printf("%s", parser.Help().c_str());
        return 0;
    } catch (args::ParseError const& e) {
        fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }

    generator::spartn::CrcType resolved_crc_type = generator::spartn::CrcType::CRC16;
    if (crc_type) {
        auto t = args::get(crc_type);
        if (t == "crc8")
            resolved_crc_type = generator::spartn::CrcType::CRC8;
        else if (t == "crc16")
            resolved_crc_type = generator::spartn::CrcType::CRC16;
        else if (t == "crc24q")
            resolved_crc_type = generator::spartn::CrcType::CRC24Q;
        else {
            fprintf(stderr, "error: --crc-type must be crc8, crc16, or crc24q\n");
            return 1;
        }
    }

    // Initialize logging
    loglet::initialize();
    loglet::set_color_enable(true);
    if (trace) {
        loglet::set_level(loglet::Level::Trace);
    } else if (debug) {
        loglet::set_level(loglet::Level::Debug);
    } else if (verbose) {
        loglet::set_level(loglet::Level::Verbose);
    } else {
        loglet::set_level(loglet::Level::Warning);
    }

    // Read input
    std::vector<uint8_t> data;
    if (input_file) {
        data = read_file(args::get(input_file));
    } else {
        data = read_stdin();
    }

    if (data.empty()) {
        fprintf(stderr, "error: no input data\n");
        return 1;
    }

    // Configure generator
    generator::spartn::Generator gen;
    gen.set_gps_supported(gps);
    gen.set_glonass_supported(glonass);
    gen.set_galileo_supported(galileo);
    gen.set_beidou_supported(beidou);
    gen.set_qzss_supported(qzss);
    gen.set_navic_supported(navic);
    gen.set_generate_ocb(!no_ocb);
    gen.set_generate_hpac(!no_hpac);
    gen.set_generate_gad(!no_gad);
    gen.set_do_not_use_satellite(!no_dnu);
    gen.set_continuity_indicator(320.0);

    // Open output
    FILE* out = stdout;
    if (output_file) {
        out = fopen(args::get(output_file).c_str(), hex_output ? "w" : "wb");
        if (!out) {
            fprintf(stderr, "error: cannot open output file: %s\n", args::get(output_file).c_str());
            return 1;
        }
    }

    // Process messages
    size_t offset       = 0;
    size_t msg_count    = 0;
    size_t spartn_count = 0;
    size_t total_size   = data.size();
    size_t msg_limit    = limit ? args::get(limit) : SIZE_MAX;
    int    last_pct     = -1;

    auto start_time  = std::chrono::steady_clock::now();
    auto last_update = start_time;

    while (offset < data.size() && msg_count < msg_limit) {
        int  pct = static_cast<int>((offset * 100) / total_size);
        auto now = std::chrono::steady_clock::now();

        if (pct != last_pct &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update).count() >=
                100) {
            auto elapsed =
                std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
            auto estimated_total = (elapsed * 100) / (pct > 0 ? pct : 1);
            auto remaining       = estimated_total - elapsed;

            fprintf(stderr, "\rProcessing: %d%% (elapsed: %lds, remaining: ~%lds)", pct, elapsed,
                    remaining);
            last_pct    = pct;
            last_update = now;
        }

        size_t consumed = 0;
        auto   lpp      = decode_lpp(data.data() + offset, data.size() - offset, &consumed);

        if (!lpp) {
            if (consumed > 0) {
                offset += consumed;
                continue;
            }
            break;
        }

        msg_count++;
        auto messages = gen.generate(lpp);
        ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp);

        for (auto& msg : messages) {
            msg.set_crc_type(resolved_crc_type);
            if (solution_id) msg.set_solution_id(static_cast<uint8_t>(args::get(solution_id)));
            if (solution_processor_id)
                msg.set_solution_processor_id(
                    static_cast<uint8_t>(args::get(solution_processor_id)));
            auto bytes = msg.build();
            TRACEF("%02X %02X: %zu bytes\n", msg.message_type(), msg.message_subtype(),
                   bytes.size());
            spartn_count++;

            if (hex_output) {
                for (auto b : bytes) {
                    fprintf(out, "%02X", b);
                }
                fprintf(out, "\n");
            } else {
                fwrite(bytes.data(), 1, bytes.size(), out);
            }
        }

        offset += consumed;
    }

    fprintf(stderr, "\rProcessing: 100%%                                        \n");

    if (out != stdout) {
        fclose(out);
    }

    fprintf(stderr, "Processed %zu LPP messages, generated %zu SPARTN messages\n", msg_count,
            spartn_count);

    auto const& stats = gen.statistics();
    if (!stats.message_counts.empty()) {
        fprintf(stderr, "\nSPARTN message counts:\n");
        for (auto const& entry : stats.message_counts) {
            uint8_t type    = (entry.first >> 8) & 0xFF;
            uint8_t subtype = entry.first & 0xFF;
            fprintf(stderr, "  type=%u subtype=%u: %zu\n", type, subtype, entry.second);
        }
    }

    if (!stats.lpp_ie_counts.empty()) {
        fprintf(stderr, "\nLPP IE counts:\n");
        for (auto const& entry : stats.lpp_ie_counts) {
            fprintf(stderr, "  %s: %zu\n", entry.first.c_str(), entry.second);
        }
    }

    if (!stats.lpp_ie_per_gnss.empty()) {
        fprintf(stderr, "\nLPP IE per GNSS:\n");
        for (auto const& ie_entry : stats.lpp_ie_per_gnss) {
            fprintf(stderr, "  %s:\n", ie_entry.first.c_str());
            for (auto const& gnss_entry : ie_entry.second) {
                fprintf(stderr, "    GNSS %ld: %zu\n", gnss_entry.first, gnss_entry.second);
            }
        }
    }

    return 0;
}
