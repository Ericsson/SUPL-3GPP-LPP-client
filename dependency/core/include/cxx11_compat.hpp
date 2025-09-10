#pragma once

// C++17 if constexpr polyfill for C++11
#if __cplusplus < 201703L
#define CONSTEXPR_IF if
#else
#define CONSTEXPR_IF if constexpr
#endif

#ifdef USE_CXX11_COMPAT

#include <memory>
#include <utility>
#include <string>
#include <sys/stat.h>

namespace std {

template<typename T, typename... Args>
::std::unique_ptr<T> make_unique(Args&&... args) {
    return ::std::unique_ptr<T>(new T(::std::forward<Args>(args)...));
}

}

#endif

// C++17 std::filesystem::create_directories polyfill
#if __cplusplus >= 201703L
#if __has_include(<filesystem>)
#include <filesystem>
#define HAS_CREATE_DIRECTORIES 1
inline void create_directories_compat(const ::std::string& path) {
    ::std::filesystem::create_directories(path);
}
#endif
#endif

#ifndef HAS_CREATE_DIRECTORIES
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#define HAS_CREATE_DIRECTORIES 1
inline void create_directories_compat(const ::std::string& path) {
    ::std::string::size_type pos = 0;
    while ((pos = path.find('/', pos)) != ::std::string::npos) {
        ::std::string dir = path.substr(0, pos++);
        if (!dir.empty()) mkdir(dir.c_str(), 0755);
    }
    mkdir(path.c_str(), 0755);
}
#endif