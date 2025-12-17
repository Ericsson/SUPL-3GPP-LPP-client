#pragma once
#include <io/stream.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace io {

class StreamRegistry {
public:
    void add(std::string const& id, std::shared_ptr<Stream> stream) NOEXCEPT;

    NODISCARD std::shared_ptr<Stream> get(std::string const& id) const NOEXCEPT;
    NODISCARD bool                    has(std::string const& id) const NOEXCEPT;

    bool schedule_all(scheduler::Scheduler& scheduler) NOEXCEPT;
    void cancel_all() NOEXCEPT;

private:
    std::unordered_map<std::string, std::shared_ptr<Stream>> mStreams;
};

}  // namespace io
