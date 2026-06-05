#pragma once
#include <chrono>
#include <fstream>
#include <memory>

#include <io/output.hpp>

class ChunkedLogOutput : public io::Output {
public:
    EXPLICIT ChunkedLogOutput(std::string const& base_path) NOEXCEPT;
    ~ChunkedLogOutput() NOEXCEPT override;

    void                  write(uint8_t const* buffer, size_t length) NOEXCEPT override;
    NODISCARD char const* name() const NOEXCEPT override { return "chunked-log"; }

protected:
    NODISCARD bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;
    NODISCARD bool do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    void        ensure_current_file() NOEXCEPT;
    std::string generate_filename() NOEXCEPT;

    std::string                           mBasePath;
    std::unique_ptr<std::ofstream>        mCurrentFile;
    std::chrono::system_clock::time_point mCurrentHour;
};
