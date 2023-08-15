#pragma once
#include <interface/types.hpp>
#include <string>

namespace interface {

/// @brief Data bits.
enum class DataBits {
    /// @brief Five (5) data bits.
    FIVE,
    /// @brief Six (6) data bits.
    SIX,
    /// @brief Seven (7) data bits.
    SEVEN,
    /// @brief Eight (8) data bits.
    EIGHT,
};

/// @brief Stop bits.
enum class StopBits {
    /// @brief One stop bit.
    ONE,
    /// @brief Two stop bits.
    TWO,
};

/// @brief Parity bit.
enum class ParityBit {
    /// @brief No parity bit.
    NONE,
    /// @brief Odd parity bit.
    ODD,
    /// @brief Even parity bit.
    EVEN,
};

/// @brief Interface for reading and writing data.
class Interface {
public:
    IF_EXPLICIT Interface() IF_NOEXCEPT = default;
    virtual ~Interface() IF_NOEXCEPT    = default;

    Interface(const Interface&)            = delete;
    Interface& operator=(const Interface&) = delete;
    Interface(Interface&&)                 = delete;
    Interface& operator=(Interface&&)      = delete;

    /// @brief Open the interface.
    virtual void open() = 0;
    /// @brief Close the interface.
    virtual void close() = 0;

    /// @brief Read data from the interface.
    /// @return The number of bytes read.
    virtual size_t read(void* data, size_t length) = 0;
    /// @brief Write data to the interface.
    /// @return The number of bytes written.
    virtual size_t write(const void* data, size_t length) = 0;

    /// @brief Check if the interface can read (i.e. if there is data available to read).
    IF_NODISCARD virtual bool can_read() IF_NOEXCEPT = 0;
    /// @brief Check if the interface can write (i.e. if there is space available to write).
    IF_NODISCARD virtual bool can_write() IF_NOEXCEPT = 0;

    /// @brief Block until the interface can read.
    virtual void wait_for_read() IF_NOEXCEPT = 0;
    /// @brief Block until the interface can write.
    virtual void wait_for_write() IF_NOEXCEPT = 0;

    /// @brief Check if the interface is open.
    IF_NODISCARD virtual bool is_open() IF_NOEXCEPT = 0;

    /// @brief Output interface information to stdout.
    virtual void print_info() IF_NOEXCEPT = 0;

public:
    /// @brief Create a file interface.
    /// @param file_path The path to the file.
    /// @param truncate Whether to truncate the file.
    static Interface* file(std::string file_path, bool truncate);

    /// @brief Create a serial interface.
    /// @param device_path The path to the serial device.
    /// @param baud_rate The baud rate.
    /// @param data_bits The number of data bits.
    /// @param stop_bits The number of stop bits.
    /// @param parity_bit The parity bit.
    static Interface* serial(std::string device_path, uint32_t baud_rate, DataBits data_bits,
                             StopBits stop_bits, ParityBit parity_bit);

    /// @brief Create an I2C interface.
    /// @param device_path The path to the I2C device.
    /// @param address The I2C address.
    static Interface* i2c(std::string device_path, uint8_t address);

    /// @brief Create a TCP interface.
    /// @param host The host to connect to (resolved using getaddrinfo).
    /// @param port The port to connect to.
    /// @param reconnect Whether to reconnect if the connection is lost.
    static Interface* tcp(std::string host, uint16_t port, bool reconnect);

    /// @brief Create a UDP interface.
    /// @param host The host to connect to (resolved using getaddrinfo).
    /// @param port The port to connect to.
    /// @param reconnect Whether to reconnect if the connection is lost.
    static Interface* udp(std::string host, uint16_t port, bool reconnect);

    /// @brief Create a stdout interface.
    static Interface* stdout();
};

}  // namespace interface
