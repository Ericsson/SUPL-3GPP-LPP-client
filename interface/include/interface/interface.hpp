#pragma once
#include <interface/types.hpp>
#include <string>

namespace interface {

enum class StopBits {
    One,
    Two,
};

enum class ParityBits {
    None,
    Odd,
    Even,
};

class Interface {
public:
    IF_EXPLICIT Interface() IF_NOEXCEPT = default;
    virtual ~Interface() IF_NOEXCEPT = default;

    Interface(const Interface&)            = delete;
    Interface& operator=(const Interface&) = delete;
    Interface(Interface&&)                 = delete;
    Interface& operator=(Interface&&)      = delete;

    virtual void open()  = 0;
    virtual void close() = 0;

    virtual size_t read(void* data, size_t length)        = 0;
    virtual size_t write(const void* data, size_t length) = 0;

    IF_NODISCARD virtual bool can_read() const IF_NOEXCEPT = 0;
    IF_NODISCARD virtual bool can_write() const IF_NOEXCEPT = 0;

    static Interface* file(std::string file_path, bool truncate);
    static Interface* serial(std::string device_path, uint32_t baud_rate, StopBits stop_bits,
                             ParityBits parity_bits);
    static Interface* i2c(std::string device_path, uint8_t address);
    static Interface* tcp(std::string host, uint16_t port);
    static Interface* udp(std::string host, uint16_t port);
    static Interface* stdout();
};

}  // namespace interface
