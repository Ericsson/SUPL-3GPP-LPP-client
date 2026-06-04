#pragma once
#include <cstdint>
#include <cstdio>
#include <string>

namespace format {
namespace tbin {

static constexpr char    MAGIC[4] = {'T', 'B', 'I', 'N'};
static constexpr uint8_t VERSION  = 1;

class Writer {
public:
    Writer() = default;
    ~Writer();

    bool open(std::string const& path, std::string const& stream_id);
    void write(int64_t timestamp_us, uint8_t const* data, uint32_t length);
    void close();

private:
    FILE* mFile{nullptr};
};

}  // namespace tbin
}  // namespace format
