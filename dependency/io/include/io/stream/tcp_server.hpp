#pragma once
#include <io/stream.hpp>
#include <io/write_buffer.hpp>
#include <scheduler/file_descriptor.hpp>

#include <memory>
#include <string>
#include <vector>

namespace scheduler {
class SocketListenerTask;
}  // namespace scheduler

namespace io {

class TcpServerStream : public Stream {
public:
    TcpServerStream(std::string id, std::unique_ptr<scheduler::SocketListenerTask> listener,
                    ReadBufferConfig read_config = {}) NOEXCEPT;
    ~TcpServerStream() NOEXCEPT override;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) override;
    bool           cancel() override;
    void           write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD uint16_t port() const NOEXCEPT;

private:
    struct Client {
        Client(TcpServerStream& server, int fd) NOEXCEPT;
        ~Client() NOEXCEPT;

        void write(uint8_t const* data, size_t length) NOEXCEPT;
        void destroy() NOEXCEPT;

        int fd() const NOEXCEPT { return mFd; }

    private:
        void on_read() NOEXCEPT;
        void on_write() NOEXCEPT;
        void on_error() NOEXCEPT;

        TcpServerStream&                   mServer;
        int                                mFd;
        scheduler::OwnedFileDescriptorTask mTask;
        WriteBuffer                        mWriteBuffer;
        bool                               mWriteRegistered = false;
        bool                               mDestroying      = false;
    };

    void remove_client(int fd) NOEXCEPT;

    std::unique_ptr<scheduler::SocketListenerTask> mListenerTask;
    std::vector<std::unique_ptr<Client>>           mClients;
    uint8_t                                        mReadBuf[4096];
};

}  // namespace io
