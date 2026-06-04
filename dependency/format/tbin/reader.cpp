#include <cstring>
#include <format/tbin/reader.hpp>

namespace format {
namespace tbin {

Reader::~Reader() {
    close();
}

bool Reader::open(std::string const& path) {
    mFile = fopen(path.c_str(), "rb");
    if (!mFile) return false;

    char magic[4];
    if (fread(magic, 1, 4, mFile) != 4) return false;
    if (memcmp(magic, "TBIN", 4) != 0) return false;

    uint8_t version;
    if (fread(&version, 1, 1, mFile) != 1) return false;
    if (version != 1) return false;

    uint8_t len;
    if (fread(&len, 1, 1, mFile) != 1) return false;
    mStreamId.resize(len);
    if (fread(&mStreamId[0], 1, len, mFile) != len) return false;

    return true;
}

bool Reader::next(Message& msg) {
    if (!mFile || mEof) return false;

    int64_t ts;
    if (fread(&ts, sizeof(ts), 1, mFile) != 1) {
        mEof = true;
        return false;
    }

    uint32_t length;
    if (fread(&length, sizeof(length), 1, mFile) != 1) {
        mEof = true;
        return false;
    }

    msg.timestamp_us = ts;
    msg.data.resize(length);
    if (fread(msg.data.data(), 1, length, mFile) != length) {
        mEof = true;
        return false;
    }

    return true;
}

void Reader::close() {
    if (mFile) {
        fclose(mFile);
        mFile = nullptr;
    }
}

}  // namespace tbin
}  // namespace format
