#pragma once
#include <io/stream.hpp>
#include <io/write_buffer.hpp>

#include <memory>
#include <string>

namespace scheduler {
class TcpConnectTask;
}

namespace io {

struct TcpClientConfig {
    std::string      host;
    uint16_t         port = 0;
    std::string      path;
    bool             reconnect   = false;
    ReadBufferConfig read_config = {};
};

class TcpClientStream : public Stream {
public:
    EXPLICIT TcpClientStream(std::string id, TcpClientConfig config) NOEXCEPT;
    ~TcpClientStream() NOEXCEPT override;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) override;
    bool           cancel() override;
    void           write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::string const& host() const NOEXCEPT { return mConfig.host; }
    NODISCARD uint16_t           port() const NOEXCEPT { return mConfig.port; }
    NODISCARD std::string const& path() const NOEXCEPT { return mConfig.path; }
    NODISCARD size_t pending_writes() const NOEXCEPT override { return mWriteBuffer.size(); }

private:
    TcpClientConfig mConfig;

    std::unique_ptr<scheduler::TcpConnectTask> mConnectTask;
    WriteBuffer                                mWriteBuffer;
    bool                                       mWriteRegistered = false;
    uint8_t                                    mReadBuf[4096];
};

}  // namespace io
