#include <cstring>
#include <format/tbin/writer.hpp>

namespace format {
namespace tbin {

Writer::~Writer() {
    close();
}

bool Writer::open(std::string const& path, std::string const& stream_id) {
    mFile = fopen(path.c_str(), "wb");
    if (!mFile) return false;

    fwrite(MAGIC, 1, 4, mFile);
    fwrite(&VERSION, 1, 1, mFile);
    auto len = static_cast<uint8_t>(stream_id.size());
    fwrite(&len, 1, 1, mFile);
    fwrite(stream_id.data(), 1, len, mFile);
    return true;
}

void Writer::write(int64_t timestamp_us, uint8_t const* data, uint32_t length) {
    if (!mFile) return;
    fwrite(&timestamp_us, sizeof(timestamp_us), 1, mFile);
    fwrite(&length, sizeof(length), 1, mFile);
    fwrite(data, 1, length, mFile);
}

void Writer::close() {
    if (mFile) {
        fclose(mFile);
        mFile = nullptr;
    }
}

}  // namespace tbin
}  // namespace format
