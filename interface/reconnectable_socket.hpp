#pragma once
#include "socket.hpp"
#include "types.hpp"

namespace interface {

/// Helper class to manage socket and reconnect on error.
class ReconnectableSocket {
public:
    IF_EXPLICIT ReconnectableSocket() IF_NOEXCEPT;
    IF_EXPLICIT ReconnectableSocket(int file_descriptor, NetworkAddress address,
                                    bool should_reconnect) IF_NOEXCEPT;

    void close() IF_NOEXCEPT;

    IF_NODISCARD bool can_read() IF_NOEXCEPT;
    IF_NODISCARD bool can_write() IF_NOEXCEPT;

    bool wait_for_read() IF_NOEXCEPT;
    bool wait_for_write() IF_NOEXCEPT;

    IF_NODISCARD size_t read(void* data, size_t length) IF_NOEXCEPT;
    IF_NODISCARD size_t write(const void* data, size_t length) IF_NOEXCEPT;

    IF_NODISCARD bool is_open() const IF_NOEXCEPT;
    void              try_reconnect() IF_NOEXCEPT;
    void              print_info() IF_NOEXCEPT;

    static IF_NODISCARD ReconnectableSocket connect(NetworkAddress address,
                                                    bool           should_reconnect) IF_NOEXCEPT;

private:
    Socket         mSocket;
    bool           mShouldReconnect;
    NetworkAddress mAddress;
};

}  // namespace interface
