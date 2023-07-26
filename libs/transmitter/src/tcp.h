#pragma once
#include "target.h"

#include <string>

class TcpTarget final : public TransmitterTarget {
public:
    TcpTarget(std::string ip_address, const int port);
    ~TcpTarget();

    void transmit(const void* data, const size_t size) override;
    void connect();
    
private:
    std::string mIpAddress;
    int         mPort;
    int         mSocket    = -1;
    bool        mConnected = false;
};
