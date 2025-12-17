#pragma once
#include <io/stream.hpp>
#include <io/write_buffer.hpp>

#include <memory>
#include <string>
#include <sys/socket.h>

namespace scheduler {
class SocketTask;
}

namespace io {

struct UdpClientConfig {
    std::string      host;
    uint16_t         port = 0;
    std::string      path;
    ReadBufferConfig read_config = {};
};

class UdpClientStream : public Stream {
public:
    EXPLICIT UdpClientStream(std::string id, UdpClientConfig config) NOEXCEPT;
    ~UdpClientStream() NOEXCEPT override;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) override;
    bool           cancel() override;
    void           write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::string const& host() const NOEXCEPT { return mConfig.host; }
    NODISCARD uint16_t           port() const NOEXCEPT { return mConfig.port; }
    NODISCARD std::string const& path() const NOEXCEPT { return mConfig.path; }
    NODISCARD size_t pending_writes() const NOEXCEPT override { return mWriteBuffer.size(); }

private:
    UdpClientConfig mConfig;

    int                                    mFd = -1;
    std::unique_ptr<scheduler::SocketTask> mSocketTask;
    WriteBuffer                            mWriteBuffer;
    bool                                   mWriteRegistered = false;
    uint8_t                                mReadBuf[65535];
};

}  // namespace io
