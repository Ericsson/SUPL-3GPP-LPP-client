#pragma once

#include <memory>

struct ULP_PDU;
struct OCTET_STRING;

namespace supl {

template <typename T>
struct Asn1Deleter {
    void operator()(T* ptr);
};

template <typename T>
using Asn1Unique = std::unique_ptr<T, Asn1Deleter<T>>;

using Message        = Asn1Unique<ULP_PDU>;
using EncodedMessage = Asn1Unique<OCTET_STRING>;

}  // namespace supl
