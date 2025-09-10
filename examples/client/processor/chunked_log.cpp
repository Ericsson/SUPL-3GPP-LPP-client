#include "chunked_log.hpp"
#include <cxx11_compat.hpp>

#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(p, chunked_log);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, chunked_log)

ChunkedLogOutput::ChunkedLogOutput(std::string const& base_path) NOEXCEPT
    : mBasePath(base_path), mCurrentHour{} {
    FUNCTION_SCOPE();
}

ChunkedLogOutput::~ChunkedLogOutput() NOEXCEPT {
    FUNCTION_SCOPE();
    if (mCurrentFile) {
        mCurrentFile->close();
    }
}

bool ChunkedLogOutput::do_schedule(scheduler::Scheduler&) NOEXCEPT {
    return true;
}

bool ChunkedLogOutput::do_cancel(scheduler::Scheduler&) NOEXCEPT {
    return true;
}

void ChunkedLogOutput::write(uint8_t const* buffer, size_t length) NOEXCEPT {
    FUNCTION_SCOPE();
    
    ensure_current_file();
    
    if (mCurrentFile && mCurrentFile->is_open()) {
        mCurrentFile->write(reinterpret_cast<char const*>(buffer), length);
        mCurrentFile->flush();
    }
}

void ChunkedLogOutput::ensure_current_file() NOEXCEPT {
    auto now = std::chrono::system_clock::now();
    auto current_hour = std::chrono::time_point_cast<std::chrono::hours>(now);
    
    if (current_hour != mCurrentHour || !mCurrentFile || !mCurrentFile->is_open()) {
        if (mCurrentFile) {
            mCurrentFile->close();
        }
        
        mCurrentHour = current_hour;
        auto filename = generate_filename();
        
        auto last_slash = filename.find_last_of('/');
        if (last_slash != std::string::npos) {
            create_directories_compat(filename.substr(0, last_slash));
        }
        
        mCurrentFile = std::make_unique<std::ofstream>(filename, std::ios::binary | std::ios::app);
        if (!mCurrentFile->is_open()) {
            ERRORF("Failed to open log file: %s", filename.c_str());
        }
    }
}

std::string ChunkedLogOutput::generate_filename() NOEXCEPT {
    auto time_t = std::chrono::system_clock::to_time_t(mCurrentHour);
    auto tm = *std::gmtime(&time_t);
    
    char time_buffer[32];
    strftime(time_buffer, sizeof(time_buffer), "%Y%m%d_%H%M%S", &tm);
    
    std::ostringstream oss;
    oss << mBasePath << "_" << time_buffer << ".log";
    return oss.str();
}