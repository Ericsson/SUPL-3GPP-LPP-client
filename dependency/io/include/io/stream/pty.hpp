#pragma once
#include <io/stream.hpp>
#include <io/write_buffer.hpp>

#include <memory>
#include <string>

namespace scheduler {
class SocketTask;
}

namespace io {

struct PtyConfig {
    std::string      link_path;
    bool             raw         = false;
    ReadBufferConfig read_config = {};
};

class PtyStream : public Stream {
public:
    EXPLICIT PtyStream(std::string id, PtyConfig config) NOEXCEPT;
    ~PtyStream() NOEXCEPT override;

    NODISCARD bool schedule(scheduler::Scheduler& scheduler) override;
    bool           cancel() override;
    void           write(uint8_t const* buffer, size_t length) NOEXCEPT override;

    NODISCARD std::string const& slave_path() const NOEXCEPT { return mSlavePath; }
    NODISCARD std::string const& link_path() const NOEXCEPT { return mConfig.link_path; }
    NODISCARD size_t pending_writes() const NOEXCEPT override { return mWriteBuffer.size(); }

private:
    bool configure_termios() NOEXCEPT;

    PtyConfig   mConfig;
    std::string mSlavePath;
    int         mMasterFd = -1;

    std::unique_ptr<scheduler::SocketTask> mSocketTask;
    WriteBuffer                            mWriteBuffer;
    bool                                   mWriteRegistered = false;
    uint8_t                                mReadBuf[4096];
};

}  // namespace io
