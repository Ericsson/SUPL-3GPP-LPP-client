#pragma once
#include <interface/interface.hpp>
#include <modem/types.hpp>

#include <memory>
#include <string>
#include <vector>

namespace modem {

struct Response {
    std::vector<std::string> lines;
    bool                     success;
};

struct SupportedCregModes {
    bool mode_0;
    bool mode_1;
    bool mode_2;
    bool mode_3;
};

struct Cimi {
    uint64_t imsi;
};

struct Creg {
    int      mode;
    int      status;
    uint32_t lac;
    uint32_t ci;
    int      act;

    bool is_gsm() const { return act >= 0 && act < 7; }
    bool is_lte() const { return act >= 7 && act < 11; }
};

struct Cops {
    int mode;
    int format;
    int mcc;
    int mnc;
    int act;
};

struct SupportedCopsModes {};

class Device {
public:
    MODEM_EXPLICIT Device(std::unique_ptr<interface::Interface> interface) MODEM_NOEXCEPT;

    void disable_echo();
    void enable_echo();

    void               get_cgmi();
    Cimi               get_cimi();
    Creg               get_creg();
    void               set_creg(int mode);
    SupportedCregModes list_creg();
    Cops               get_cops();
    void               set_cops_format(int format);
    SupportedCopsModes list_cops();

protected:
    void     send_requst(char const* request);
    Response wait_for_response();

private:
    std::unique_ptr<interface::Interface> mInterface;
};

}  // namespace modem
