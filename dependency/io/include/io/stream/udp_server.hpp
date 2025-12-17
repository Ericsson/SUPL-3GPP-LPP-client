#pragma once
#include <io/stream.hpp>

#include <memory>
#include <string>
#include <sys/socket.h>

namespace scheduler {
class UdpListenerTask;
}

namespace io {

struct UdpServerConfig {
    std::string      listen;
    uint16_t         port = 0;
    std::string      path;
    ReadBufferConfig read_config = {};
};

class UdpServerStream : public Stream {
public:
    EXPLICIT UdpServerStream(std::string id, UdpServerConfig config) NOEXCEPT;
    ~UdpServerStream() NOEXCEPT override;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) override;
    bool           cancel() override;
    void           write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::string const& listen_addr() const NOEXCEPT { return mConfig.listen; }
    NODISCARD uint16_t           port() const NOEXCEPT { return mConfig.port; }
    NODISCARD uint16_t           actual_port() const NOEXCEPT;
    NODISCARD std::string const& path() const NOEXCEPT { return mConfig.path; }

private:
    UdpServerConfig mConfig;

    std::unique_ptr<scheduler::UdpListenerTask> mListenerTask;
    sockaddr_storage                            mLastSender{};
    socklen_t                                   mLastSenderLen = 0;
    uint8_t                                     mReadBuf[65535];
};

}  // namespace io
