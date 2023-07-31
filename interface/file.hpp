#pragma once
#include <string>
#include "interface.hpp"

namespace interface {

class FileInterface final : public Interface {
public:
    explicit FileInterface(std::string file_path, bool truncate) IF_NOEXCEPT;
    ~FileInterface() IF_NOEXCEPT override;

    void open() override;
    void close() override;

    size_t read(void* data, size_t length) override;
    size_t write(const void* data, size_t length) override;

private:
    std::string mFilePath;
    int         mFileDescriptor;
    bool        mTruncate;
};

}  // namespace interface
