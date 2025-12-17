#pragma once
#include <io/stream.hpp>
#include <io/write_buffer.hpp>

#include <memory>

namespace scheduler {
class SocketTask;
}

namespace io {

struct FdConfig {
    int              fd;
    bool             owns_fd     = false;
    ReadBufferConfig read_config = {};
};

class FdStream : public Stream {
public:
    EXPLICIT FdStream(std::string id, FdConfig config) NOEXCEPT;
    ~FdStream() NOEXCEPT override;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) override;
    bool           cancel() override;
    void           write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD int    fd() const NOEXCEPT { return mConfig.fd; }
    NODISCARD size_t pending_writes() const NOEXCEPT override { return mWriteBuffer.size(); }

private:
    FdConfig                               mConfig;
    std::unique_ptr<scheduler::SocketTask> mSocketTask;
    WriteBuffer                            mWriteBuffer;
    bool                                   mWriteRegistered = false;
    uint8_t                                mReadBuf[4096];
};

}  // namespace io
