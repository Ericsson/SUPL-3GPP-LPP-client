#pragma once
#include "target.h"

#include <string>

class UdpTarget final : public TransmitterTarget {
public:
    UdpTarget(std::string ip_address, const int port);
    ~UdpTarget();

    void transmit(const void* data, const size_t size) override;

private:
    std::string mIpAddress;
    int         mPort;
    int         mSocket = -1;
};
