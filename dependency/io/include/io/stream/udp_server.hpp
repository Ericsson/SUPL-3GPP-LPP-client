#pragma once
#include <io/stream.hpp>

#include <memory>
#include <string>
#include <sys/socket.h>

namespace scheduler {
class UdpSocketListenerTask;
}

namespace io {

class UdpServerStream : public Stream {
public:
    UdpServerStream(std::string id, std::unique_ptr<scheduler::UdpSocketListenerTask> listener,
                    ReadBufferConfig read_config = {}) NOEXCEPT;
    ~UdpServerStream() NOEXCEPT override;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) override;
    bool           cancel() override;
    void           write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD uint16_t port() const NOEXCEPT;

private:
    std::unique_ptr<scheduler::UdpSocketListenerTask> mListenerTask;
    sockaddr_storage                                  mLastSender{};
    socklen_t                                         mLastSenderLen = 0;
    uint8_t                                           mReadBuf[65535];
};

}  // namespace io
