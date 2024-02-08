#pragma once
#include <interface/types.hpp>
#include <string>

namespace interface {

/// Data bits.
enum class DataBits {
    /// Five (5) data bits.
    FIVE,
    /// Six (6) data bits.
    SIX,
    /// Seven (7) data bits.
    SEVEN,
    /// Eight (8) data bits.
    EIGHT,
};

/// Stop bits.
enum class StopBits {
    /// One stop bit.
    ONE,
    /// Two stop bits.
    TWO,
};

/// Parity bit.
enum class ParityBit {
    /// No parity bit.
    NONE,
    /// Odd parity bit.
    ODD,
    /// Even parity bit.
    EVEN,
};

/// Interface for reading and writing data.
class Interface {
public:
    IF_EXPLICIT Interface() IF_NOEXCEPT = default;
    virtual ~Interface() IF_NOEXCEPT    = default;

    Interface(const Interface&)            = delete;
    Interface& operator=(const Interface&) = delete;
    Interface(Interface&&)                 = delete;
    Interface& operator=(Interface&&)      = delete;

    /// Open the interface.
    virtual void open() = 0;
    /// Close the interface.
    virtual void close() = 0;

    /// Read data from the interface.
    /// @return The number of bytes read.
    virtual size_t read(void* data, size_t length) = 0;
    /// Write data to the interface.
    /// @return The number of bytes written.
    virtual size_t write(const void* data, size_t length) = 0;

    /// Check if the interface can read (i.e. if there is data available to read).
    IF_NODISCARD virtual bool can_read() IF_NOEXCEPT = 0;
    /// Check if the interface can write (i.e. if there is space available to write).
    IF_NODISCARD virtual bool can_write() IF_NOEXCEPT = 0;

    /// Block until the interface can read.
    virtual void wait_for_read() IF_NOEXCEPT = 0;
    /// Block until the interface can write.
    virtual void wait_for_write() IF_NOEXCEPT = 0;

    /// Check if the interface is open.
    IF_NODISCARD virtual bool is_open() IF_NOEXCEPT = 0;

    /// Output interface information to stdout.
    virtual void print_info() IF_NOEXCEPT = 0;

public:
    /// Create a file interface.
    /// @param file_path The path to the file.
    /// @param truncate Whether to truncate the file.
    static Interface* file(std::string file_path, bool truncate);

    /// Create a serial interface.
    /// @param device_path The path to the serial device.
    /// @param baud_rate The baud rate.
    /// @param data_bits The number of data bits.
    /// @param stop_bits The number of stop bits.
    /// @param parity_bit The parity bit.
    static Interface* serial(std::string device_path, uint32_t baud_rate, DataBits data_bits,
                             StopBits stop_bits, ParityBit parity_bit);

    /// Create an I2C interface.
    /// @param device_path The path to the I2C device.
    /// @param address The I2C address.
    static Interface* i2c(std::string device_path, uint8_t address);

    /// Create a TCP interface.
    /// @param host The host to connect to (resolved using getaddrinfo).
    /// @param port The port to connect to.
    /// @param reconnect Whether to reconnect if the connection is lost.
    static Interface* tcp(std::string host, uint16_t port, bool reconnect);

    /// Create a UDP interface.
    /// @param host The host to connect to (resolved using getaddrinfo).
    /// @param port The port to connect to.
    /// @param reconnect Whether to reconnect if the connection is lost.
    static Interface* udp(std::string host, uint16_t port, bool reconnect);

    /// Create a stdout interface.
    static Interface* stdout();

    /// Create a stdin interface.
    static Interface* stdin();

    /// Create a unix socket interface. The socket type is SOCK_STREAM.
    /// @param socket_path The path to the unix socket.
    /// @param reconnect Whether to reconnect if the connection is lost.
    static Interface* unix_socket_stream(std::string socket_path, bool reconnect);
};

}  // namespace interface
