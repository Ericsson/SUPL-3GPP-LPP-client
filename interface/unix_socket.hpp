#pragma once
#include <cstddef>
#include <string>
#include "interface.hpp"
#include "reconnectable_socket.hpp"

namespace interface {

class UnixSocketInterface final : public Interface {
public:
    explicit UnixSocketInterface(std::string path, bool reconnect) IF_NOEXCEPT;
    ~UnixSocketInterface() IF_NOEXCEPT override;

    void open() override;
    void close() override;

    size_t read(void* data, size_t length) override;
    size_t write(const void* data, size_t length) override;

    IF_NODISCARD bool can_read() IF_NOEXCEPT override;
    IF_NODISCARD bool can_write() IF_NOEXCEPT override;

    void wait_for_read() IF_NOEXCEPT override;
    void wait_for_write() IF_NOEXCEPT override;

    IF_NODISCARD bool is_open() IF_NOEXCEPT override;
    void              print_info() IF_NOEXCEPT override;

private:
    std::string         mPath;
    bool                mReconnect;
    ReconnectableSocket mSocket;
};

}  // namespace interface
