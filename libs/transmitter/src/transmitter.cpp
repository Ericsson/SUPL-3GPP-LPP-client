#include <transmitter/transmitter.h>
#include "file.h"
#include "i2c.h"
#include "serial.h"
#include "stdout.h"
#include "tcp.h"
#include "udp.h"

Transmitter::Transmitter() = default;

Transmitter::~Transmitter() = default;

void Transmitter::add_i2c_target(std::string device, const uint8_t address) {
    targets.emplace_back(std::unique_ptr<TransmitterTarget>(
        reinterpret_cast<TransmitterTarget*>(new I2CTarget(device, address))));
}

void Transmitter::add_serial_target(std::string device, const int baud_rate) {
    targets.emplace_back(std::unique_ptr<TransmitterTarget>(
        reinterpret_cast<TransmitterTarget*>(new SerialTarget(device, baud_rate))));
}

void Transmitter::add_tcp_target(std::string ip_address, const int port) {
    targets.emplace_back(std::unique_ptr<TransmitterTarget>(
        reinterpret_cast<TransmitterTarget*>(new TcpTarget(ip_address, port))));
}

void Transmitter::add_udp_target(std::string ip_address, const int port) {
    targets.emplace_back(std::unique_ptr<TransmitterTarget>(
        reinterpret_cast<TransmitterTarget*>(new UdpTarget(ip_address, port))));
}

void Transmitter::add_file_target(std::string filename, bool truncate) {
    targets.emplace_back(std::unique_ptr<TransmitterTarget>(
        reinterpret_cast<TransmitterTarget*>(new FileTarget(filename, truncate))));
}

void Transmitter::add_stdout_target() {
    targets.emplace_back(std::unique_ptr<TransmitterTarget>(
        reinterpret_cast<TransmitterTarget*>(new StdoutTarget())));
}

void Transmitter::send(const void* data, const size_t size) {
    if (data == nullptr || size == 0) {
        return;
    }

    for (auto& target : targets) {
        target->transmit(data, size);
    }
}
