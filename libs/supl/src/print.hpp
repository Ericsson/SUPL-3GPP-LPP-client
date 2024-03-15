#pragma once
#include <loglet/loglet.hpp>

struct ULP_PDU;

namespace supl {

void print(loglet::Level, ULP_PDU* ulp_pdu);

}
