#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace format {
namespace tbin {

struct Message {
    int64_t              timestamp_us;
    std::vector<uint8_t> data;
};

class Reader {
public:
    Reader() = default;
    ~Reader();

    bool open(std::string const& path);
    bool next(Message& msg);
    void close();

    std::string const& stream_id() const { return mStreamId; }
    bool               eof() const { return mEof; }

private:
    FILE*       mFile{nullptr};
    std::string mStreamId;
    bool        mEof{false};
};

}  // namespace tbin
}  // namespace format
