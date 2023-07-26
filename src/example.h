#pragma once

#include <memory>
#include <string>

struct LocationServerOptions {
    std::string host;
    int         port;
    bool        ssl;
};

struct IdentityOptions {
    std::unique_ptr<unsigned long> msisdn;
    std::unique_ptr<unsigned long> imsi;
    std::unique_ptr<std::string> ipv4;
};

struct CellOptions {
    int mcc;
    int mnc;
    int tac;
    int cid;
};

struct ModemDevice {
    std::string device;
    int         baud_rate;
};

struct ModemOptions {
    std::unique_ptr<ModemDevice> device;
};

struct FileOutput {
    std::string file_path;
};

struct SerialOutput {
    std::string device;
    int         baud_rate;
};

struct I2COutput {
    std::string device;
    uint8_t     address;
};

struct TCPOutput {
    std::string ip_address;
    int         port;
};

struct UDPOutput {
    std::string ip_address;
    int         port;
};

struct StdoutOutput {};

struct OutputOptions {
    std::unique_ptr<FileOutput>   file;
    std::unique_ptr<SerialOutput> serial;
    std::unique_ptr<I2COutput>    i2c;
    std::unique_ptr<TCPOutput>    tcp;
    std::unique_ptr<UDPOutput>    udp;
    std::unique_ptr<StdoutOutput> stdout_output;
};