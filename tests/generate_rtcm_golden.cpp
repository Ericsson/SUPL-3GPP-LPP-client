#include <generator/rtcm/generator.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <LPP-Message.h>
EXTERNAL_WARNINGS_POP

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

static LPP_Message* load_uper(std::string const& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return nullptr;
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    LPP_Message*         lpp = nullptr;
    asn_dec_rval_t       rval =
        uper_decode_complete(nullptr, &asn_DEF_LPP_Message, (void**)&lpp, data.data(), data.size());
    if (rval.code != RC_OK) {
        if (lpp) ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp);
        return nullptr;
    }
    return lpp;
}

static void write_rtcm(std::string const&                           path,
                       std::vector<generator::rtcm::Message> const& messages) {
    std::ofstream out(path, std::ios::binary);
    for (auto const& msg : messages) {
        auto const& d = msg.data();
        out.write(reinterpret_cast<char const*>(d.data()), static_cast<std::streamsize>(d.size()));
    }
}

struct Config {
    char const*                    prefix;
    generator::rtcm::MessageFilter filter;
};

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <test-data-dir>\n", argv[0]);
        fprintf(stderr, "  Regenerates golden .rtcm files from existing .uper files.\n");
        return 1;
    }

    std::string base = argv[1];
    base += "/rtcm/captured";

    Config configs[] = {
        {"default-", {}},
        {"gps_only-",
         []() {
             generator::rtcm::MessageFilter f;
             f.systems.glonass = false;
             f.systems.galileo = false;
             f.systems.beidou  = false;
             return f;
         }()},
        {"msm4-",
         []() {
             generator::rtcm::MessageFilter f;
             f.msm.force_msm4 = true;
             return f;
         }()},
        {"msm7-",
         []() {
             generator::rtcm::MessageFilter f;
             f.msm.force_msm7 = true;
             return f;
         }()},
    };

    // Enumerate .uper files matching each prefix
    for (auto const& cfg : configs) {
        // Scan directory for matching files
        // Use a simple glob via popen since we have no directory API here
        std::string cmd = "ls " + base + "/" + cfg.prefix + "*.uper 2>/dev/null";
        FILE*       fp  = popen(cmd.c_str(), "r");
        if (!fp) continue;

        char line[4096];
        while (fgets(line, sizeof(line), fp)) {
            std::string uper_path = line;
            while (!uper_path.empty() && (uper_path.back() == '\n' || uper_path.back() == '\r'))
                uper_path.pop_back();

            std::string rtcm_path = uper_path.substr(0, uper_path.size() - 5) + ".rtcm";

            auto lpp = load_uper(uper_path);
            if (!lpp) {
                fprintf(stderr, "Failed to load: %s\n", uper_path.c_str());
                continue;
            }

            generator::rtcm::Generator gen;
            auto                       messages = gen.generate(lpp, cfg.filter);
            ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp);

            write_rtcm(rtcm_path, messages);
            fprintf(stdout, "Written %zu messages -> %s\n", messages.size(), rtcm_path.c_str());
        }
        pclose(fp);
    }

    return 0;
}
