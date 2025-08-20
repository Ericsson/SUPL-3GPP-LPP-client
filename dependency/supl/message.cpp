#include "supl/message.hpp"

#include <OCTET_STRING.h>
#include <ULP-PDU.h>

namespace supl {

template <>
void Asn1Deleter<ULP_PDU>::operator()(ULP_PDU* ptr) {
    if (ptr) {
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, ptr);
    }
}

template <>
void Asn1Deleter<OCTET_STRING>::operator()(OCTET_STRING* ptr) {
    if (ptr) {
        ASN_STRUCT_FREE(asn_DEF_OCTET_STRING, ptr);
    }
}

}  // namespace supl
