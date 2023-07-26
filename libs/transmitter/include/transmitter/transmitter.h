#pragma once
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class TransmitterTarget {
public:
    virtual ~TransmitterTarget() = default;

    virtual void transmit(const void* data, const size_t size) = 0;
};

class Transmitter {
public:
    Transmitter();
    ~Transmitter();

    void add_serial_target(std::string device, const int baud_rate);
    void add_i2c_target(std::string device, const uint8_t address);
    void add_tcp_target(std::string ip_address, const int port);
    void add_udp_target(std::string ip_address, const int port);
    void add_file_target(std::string filename, bool truncate);
    void add_stdout_target();

    void send(const void* data, const size_t size);

private:
    std::vector<std::unique_ptr<TransmitterTarget>> targets = {};
};
