#pragma once
#include <io/stream.hpp>
#include <io/write_buffer.hpp>

#include <memory>
#include <string>
#include <vector>

namespace scheduler {
class TcpListenerTask;
class SocketTask;
}  // namespace scheduler

namespace io {

struct TcpServerConfig {
    std::string      listen;
    uint16_t         port = 0;
    std::string      path;
    ReadBufferConfig read_config = {};
};

class TcpServerStream : public Stream {
public:
    EXPLICIT TcpServerStream(std::string id, TcpServerConfig config) NOEXCEPT;
    ~TcpServerStream() NOEXCEPT override;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) override;
    bool           cancel() override;
    void           write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::string const& listen_addr() const NOEXCEPT { return mConfig.listen; }
    NODISCARD uint16_t           port() const NOEXCEPT { return mConfig.port; }
    NODISCARD uint16_t           actual_port() const NOEXCEPT;
    NODISCARD std::string const& path() const NOEXCEPT { return mConfig.path; }

private:
    struct Client {
        int                                    fd;
        std::unique_ptr<scheduler::SocketTask> task;
        WriteBuffer                            write_buffer;
        bool                                   write_registered = false;
    };

    void remove_client(Client* client) NOEXCEPT;

    TcpServerConfig mConfig;

    std::unique_ptr<scheduler::TcpListenerTask> mListenerTask;
    std::vector<std::unique_ptr<Client>>        mClients;
    uint8_t                                     mReadBuf[4096];
};

}  // namespace io
