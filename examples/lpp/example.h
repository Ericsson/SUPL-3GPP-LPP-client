#pragma once
#include <interface/interface.hpp>
#include <memory>
#include <string>
#include <vector>

struct LocationServerOptions {
    std::string host;
    int         port;
    bool        ssl;
};

struct IdentityOptions {
    std::unique_ptr<unsigned long> msisdn;
    std::unique_ptr<unsigned long> imsi;
    std::unique_ptr<std::string>   ipv4;
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

struct OutputOptions {
    std::vector<interface::Interface*> interfaces;
};