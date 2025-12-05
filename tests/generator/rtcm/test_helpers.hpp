#pragma once
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <LPP-Message.h>
EXTERNAL_WARNINGS_POP

#include <generator/rtcm/generator.hpp>

inline LPP_Message* load_lpp_uper(std::string const& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return nullptr;

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());

    LPP_Message*   lpp = nullptr;
    asn_dec_rval_t rval =
        uper_decode_complete(nullptr, &asn_DEF_LPP_Message, (void**)&lpp, data.data(), data.size());

    if (rval.code != RC_OK) {
        if (lpp) ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp);
        return nullptr;
    }

    return lpp;
}

inline std::vector<std::vector<uint8_t>> load_rtcm_messages(std::string const& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return {};

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());

    std::vector<std::vector<uint8_t>> messages;

    size_t pos = 0;
    while (pos < data.size()) {
        if (pos + 3 > data.size()) break;

        if (data[pos] != 0xD3) {
            pos++;
            continue;
        }

        uint16_t length     = ((data[pos + 1] & 0x03) << 8) | data[pos + 2];
        size_t   frame_size = 3 + length + 3;

        if (pos + frame_size > data.size()) break;

        std::vector<uint8_t> message(data.begin() + pos, data.begin() + pos + frame_size);
        messages.push_back(message);

        pos += frame_size;
    }

    return messages;
}

inline bool compare_rtcm_messages(std::vector<generator::rtcm::Message> const& actual,
                                  std::vector<std::vector<uint8_t>> const&     expected) {
    if (actual.size() != expected.size()) return false;

    for (size_t i = 0; i < actual.size(); i++) {
        if (actual[i].data() != expected[i]) return false;
    }

    return true;
}

struct TestCase {
    std::string uper_path;
    std::string rtcm_path;
    std::string name;
};

inline std::vector<TestCase> find_test_cases(std::string const& pattern) {
    std::vector<TestCase> cases;

    auto dir    = std::filesystem::path(pattern).parent_path();
    auto prefix = std::filesystem::path(pattern).filename().string();

    size_t wildcard_pos = prefix.find('*');
    if (wildcard_pos == std::string::npos) {
        return cases;
    }

    std::string prefix_part = prefix.substr(0, wildcard_pos);
    std::string suffix_part = prefix.substr(wildcard_pos + 1);
    if (!std::filesystem::exists(dir)) {
        return cases;
    }

    for (auto const& entry : std::filesystem::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;

        auto filename = entry.path().filename().string();
        if (!filename.starts_with(prefix_part)) continue;
        if (!filename.ends_with(suffix_part)) continue;

        auto base      = filename.substr(0, filename.size() - suffix_part.size());
        auto uper_path = (dir / (base + ".uper")).string();
        auto rtcm_path = (dir / (base + ".rtcm")).string();

        if (std::filesystem::exists(uper_path) && std::filesystem::exists(rtcm_path)) {
            cases.push_back({uper_path, rtcm_path, base});
        }
    }

    std::sort(cases.begin(), cases.end(), [](auto const& a, auto const& b) {
        return a.name < b.name;
    });

    return cases;
}
