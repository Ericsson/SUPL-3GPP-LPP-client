#pragma once
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace test_utils {

inline bool file_exists(std::string const& path) {
    struct stat buffer;
    return stat(path.c_str(), &buffer) == 0;
}

inline std::string path_join(std::string const& dir, std::string const& file) {
    if (dir.empty()) return file;
    if (dir.back() == '/') return dir + file;
    return dir + "/" + file;
}

inline std::string filename(std::string const& path) {
    auto pos = path.find_last_of('/');
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

inline std::string parent_path(std::string const& path) {
    auto pos = path.find_last_of('/');
    return pos == std::string::npos ? "." : path.substr(0, pos);
}

inline bool starts_with(std::string const& str, std::string const& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

inline bool ends_with(std::string const& str, std::string const& suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline std::vector<std::string> list_directory(std::string const& path) {
    std::vector<std::string> result;
    DIR*                     dir = opendir(path.c_str());
    if (!dir) return result;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        std::string full_path = path_join(path, entry->d_name);
        struct stat st;
        if (stat(full_path.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            result.push_back(entry->d_name);
        }
    }
    closedir(dir);
    return result;
}

inline std::vector<std::string> find_files_with_suffix(char const** search_paths, size_t num_paths,
                                                       char const* suffix) {
    std::vector<std::string> files;

    for (size_t i = 0; i < num_paths; i++) {
        DIR* dir = opendir(search_paths[i]);
        if (!dir) continue;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strstr(entry->d_name, suffix) != nullptr) {
                files.push_back(path_join(search_paths[i], entry->d_name));
            }
        }
        closedir(dir);

        if (!files.empty()) break;
    }

    return files;
}

inline std::vector<std::string> find_files_with_prefix_and_suffix(char const** search_paths,
                                                                  size_t       num_paths,
                                                                  char const*  prefix,
                                                                  char const*  suffix) {
    std::vector<std::string> files;

    for (size_t i = 0; i < num_paths; i++) {
        DIR* dir = opendir(search_paths[i]);
        if (!dir) continue;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (starts_with(entry->d_name, prefix) && ends_with(entry->d_name, suffix)) {
                files.push_back(path_join(search_paths[i], entry->d_name));
            }
        }
        closedir(dir);

        if (!files.empty()) break;
    }

    return files;
}

}  // namespace test_utils
