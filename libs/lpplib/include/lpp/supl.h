#pragma once

#include "asnlib.h"
#include "tcp.h"

#define SUPL_CLIENT_RECEIVER_BUFFER_SIZE (1 << 15)

using SUPL_Message        = ASN_Unique<ULP_PDU>;
using SUPL_EncodedMessage = ASN_Unique<OCTET_STRING>;

class SUPL_Session {
public:
    static std::unique_ptr<SUPL_Session> msisdn(long id, unsigned long long msisdn,
                                                bool switch_order);
    static std::unique_ptr<SUPL_Session> imsi(long id, unsigned long long imsi, bool switch_order);
    static std::unique_ptr<SUPL_Session> ip_address(long id, const std::string& addr);
    static std::unique_ptr<SUPL_Session> ip_address(long id, uint32_t addr);

    ~SUPL_Session();

    SetSessionID* copy_set();
    SlpSessionID* copy_slp();

    void harvest(SUPL_Message& message);

private:
    SUPL_Session(SetSessionID* set, SlpSessionID* slp) : mSet(set), mSlp(slp) {}

    SetSessionID* mSet;
    SlpSessionID* mSlp;
};

class SUPL_Client {
public:
    SUPL_Client();
    ~SUPL_Client();

    void set_session(std::unique_ptr<SUPL_Session> session);

    bool connect(const std::string& host, int port, bool use_ssl);
    bool disconnect();
    bool is_connected();

    SUPL_Message receive2();
    SUPL_Message receive(int milliseconds);
    bool         send(SUPL_Message& message);
    void         harvest(SUPL_Message& message);

    SUPL_Message create_message(UlpMessage_PR present);

protected:
    SUPL_EncodedMessage encode(SUPL_Message& message);

    SUPL_Message process();

private:
    std::unique_ptr<TCP_Client>   mTCP;
    std::unique_ptr<SUPL_Session> mSession;

    // TODO(ewasjon): move to heap
    char   mReceiveBuffer[SUPL_CLIENT_RECEIVER_BUFFER_SIZE];
    size_t mReceiveLength;
};
