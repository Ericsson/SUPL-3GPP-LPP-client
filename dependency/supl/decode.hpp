#pragma once
#include <supl/session.hpp>

struct ULP_PDU;

namespace supl {

struct RESPONSE;
struct END;
struct POS;

bool decode_response(Session::SET& set, Session::SLP& slp, RESPONSE& response, ULP_PDU* pdu);
bool decode_end(Session::SET& set, Session::SLP& slp, END& end, ULP_PDU* pdu);
bool decode_pos(Session::SET& set, Session::SLP& slp, POS& pos, ULP_PDU* pdu);

}  // namespace supl
