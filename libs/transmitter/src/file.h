#pragma once
#include "target.h"

#include <string>

class FileTarget final : public TransmitterTarget {
public:
    FileTarget(std::string file_path, bool truncate);
    ~FileTarget();

    void transmit(const void* data, const size_t size) override;

private:
    std::string mFilePath;
    int         mFileDescriptor = -1;
};
