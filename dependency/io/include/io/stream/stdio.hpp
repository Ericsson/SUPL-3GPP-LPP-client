#pragma once
#include <io/stream.hpp>
#include <io/write_buffer.hpp>

#include <memory>

namespace scheduler {
class SocketTask;
}

namespace io {

struct StdioConfig {
    bool             use_stderr  = false;
    ReadBufferConfig read_config = {};
};

class StdioStream : public Stream {
public:
    EXPLICIT StdioStream(std::string id, StdioConfig config) NOEXCEPT;
    ~StdioStream() NOEXCEPT override;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) override;
    bool           cancel() override;
    void           write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD size_t pending_writes() const NOEXCEPT override { return mWriteBuffer.size(); }

private:
    StdioConfig mConfig;

    std::unique_ptr<scheduler::SocketTask> mSocketTask;
    std::unique_ptr<scheduler::SocketTask> mWriteTask;
    WriteBuffer                            mWriteBuffer;
    bool                                   mWriteRegistered = false;
    uint8_t                                mReadBuf[4096];
};

}  // namespace io
