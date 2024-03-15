#include "lpp/message.hpp"

#include <LPP-Message.h>

namespace lpp {
namespace custom {

template <>
void Deleter<LPP_Message>::operator()(LPP_Message* ptr) {
    if (ptr) {
        ASN_STRUCT_FREE(asn_DEF_LPP_Message, ptr);
    }
}

}  // namespace custom
}  // namespace lpp
