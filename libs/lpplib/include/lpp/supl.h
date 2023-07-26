#pragma once

#include "asnlib.h"
#include "tcp.h"

using SUPL_Message        = ASN_Unique<ULP_PDU>;
using SUPL_EncodedMessage = ASN_Unique<OCTET_STRING>;

class SUPL_Session {
public:
    static std::unique_ptr<SUPL_Session> msisdn(long id, unsigned long msisdn);
    static std::unique_ptr<SUPL_Session> imsi(long id, unsigned long imsi);
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

    SUPL_Message receive(int milliseconds);
    bool         send(SUPL_Message& message);
    void         harvest(SUPL_Message& message);

    SUPL_Message create_message(UlpMessage_PR present);

protected:
    SUPL_EncodedMessage encode(SUPL_Message& message);

private:
    std::unique_ptr<TCP_Client>   mTCP;
    std::unique_ptr<SUPL_Session> mSession;

    // TODO(ewasjon): move to heap
    char mReceiveBuffer[1 << 16];
};

#if 0
#include <cell_id.h>

struct SUPLSession {
    int           id;
    int           slp_size;
    char*         slp_buffer;
    unsigned char ip[4];
};

void supl_session_create(SUPLSession* session, uint32_t ip);

int supl_session_harvest(SUPLSession* session, ULP_PDU_t* ulp);

// Helper function for creating a SUPL message.
ULP_PDU_t* supl_create_message(SUPLSession* session, UlpMessage_PR present);

void supl_free_message(ULP_PDU_t* ulp);

// Helper function to send a SUPL message.
int supl_send_pdu(TCPClient* client, ULP_PDU_t* ulp);

// Helper to read SUPL messages
int supl_read(TCPClient* client, ULP_PDU_t* pdu);

ULP_PDU_t* supl_read_pdu(TCPClient* client, int milliseconds);

// SUPL START message
int supl_start(TCPClient* client, CellID cell, SUPLSession* session);

// Helper function for wait/receiving a SUPL response
int supl_response(TCPClient* client, SUPLSession* session);

// Helper function to send SUPL POSINIT with LPP messages.
int supl_send_posinit(TCPClient* client, CellID cell, SUPLSession* session,
                      OCTET_STRING** lpp_messages, int lpp_message_count);

// Helper function for receiving SUPL POS responses.
int supl_receive_pos(TCPClient* client, LPP_Message** messages, int* count,
                     int messages_size, int milliseconds);

// Helper function for sending a SUPL POS message with LPP payloads.
int supl_send_pos(TCPClient* client, SUPLSession* session,
                  OCTET_STRING* lpp_message);
#endif