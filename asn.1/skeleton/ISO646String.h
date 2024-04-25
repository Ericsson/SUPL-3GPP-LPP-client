/*-
 * Copyright (c) 2003-2017 Lev Walkin <vlm@lionet.info>. All rights reserved.
 * Redistribution and modifications are permitted subject to BSD license.
 */
#ifndef	_ISO646String_H_
#define	_ISO646String_H_

#include <asn_application.h>
#include <VisibleString.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef VisibleString_t ISO646String_t;	/* Implemented using VisibleString */

extern asn_TYPE_descriptor_t asn_DEF_ISO646String;
extern asn_TYPE_operation_t asn_OP_ISO646String;

#define ISO646String_free OCTET_STRING_free

#if !defined(ASN_DISABLE_PRINT_SUPPORT)
#define ISO646String_print OCTET_STRING_print_utf8
#endif  /* !defined(ASN_DISABLE_PRINT_SUPPORT) */

#define ISO646String_compare OCTET_STRING_compare
#define ISO646String_copy    OCTET_STRING_copy   

#define ISO646String_constraint VisibleString_constraint

#if !defined(ASN_DISABLE_BER_SUPPORT)
#define ISO646String_decode_ber OCTET_STRING_decode_ber
#define ISO646String_encode_der OCTET_STRING_encode_der
#endif  /* !defined(ASN_DISABLE_BER_SUPPORT) */

#if !defined(ASN_DISABLE_XER_SUPPORT)
#define ISO646String_decode_xer OCTET_STRING_decode_xer_utf8
#define ISO646String_encode_xer OCTET_STRING_encode_xer_utf8
#endif  /* !defined(ASN_DISABLE_XER_SUPPORT) */

#if !defined(ASN_DISABLE_JER_SUPPORT)
#define ISO646String_decode_jer OCTET_STRING_decode_jer_utf8
#define ISO646String_encode_jer OCTET_STRING_encode_jer_utf8
#endif  /* !defined(ASN_DISABLE_JER_SUPPORT) */

#if !defined(ASN_DISABLE_OER_SUPPORT)
#define ISO646String_decode_oer OCTET_STRING_decode_oer
#define ISO646String_encode_oer OCTET_STRING_encode_oer
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */

#if !defined(ASN_DISABLE_UPER_SUPPORT)
#define ISO646String_decode_uper OCTET_STRING_decode_uper
#define ISO646String_encode_uper OCTET_STRING_encode_uper
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) */
#if !defined(ASN_DISABLE_APER_SUPPORT)
#define ISO646String_decode_aper OCTET_STRING_decode_aper
#define ISO646String_encode_aper OCTET_STRING_encode_aper
#endif  /* !defined(ASN_DISABLE_APER_SUPPORT) */

#if !defined(ASN_DISABLE_RFILL_SUPPORT)
#define ISO646String_random_fill OCTET_STRING_random_fill
#endif  /* !defined(ASN_DISABLE_RFILL_SUPPORT) */

#ifdef __cplusplus
}
#endif

#endif	/* _ISO646String_H_ */
