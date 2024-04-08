#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#include <ApplicationID.h>
#include <BIT_STRING.h>
#include <CellIdentity.h>
#include <ECGI.h>
#include <GNSS-SignalID.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
#include <MCC-MNC-Digit.h>
#include <MCC.h>
#include <MNC.h>
#include <RequestAssistanceData-r9-IEs.h>
#include <RequestAssistanceData.h>
#include <TrackingAreaCode.h>
#include <ULP-PDU.h>
#include <Ver2-SUPL-START-extension.h>
#include <LPP-TransactionID.h>
#pragma GCC diagnostic pop

typedef BIT_STRING_t   BIT_STRING;
typedef OCTET_STRING_t OCTET_STRING;

typedef MCC_MNC_Digit_t MCC_MNC_Digit;
typedef MCC_t           MCC;
typedef MNC_t           MNC;
typedef ECGI_t          ECGI;

typedef LPP_Message_t       LPP_Message;
typedef LPP_TransactionID_t LPP_TransactionID;

#define OPTIONAL_MISSING NULL
#define ALLOC_ZERO(T) (reinterpret_cast<T*>(calloc(1, sizeof(T))))

#include <memory>

template <typename T>
struct ASN_Deleter {
    void operator()(T* ptr);
};

template <typename T>
using ASN_Unique = std::unique_ptr<T, ASN_Deleter<T>>;
