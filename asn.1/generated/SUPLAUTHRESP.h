/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SUPL-AUTH-RESP"
 * 	found in "src/SUPL.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_SUPLAUTHRESP_H_
#define	_SUPLAUTHRESP_H_


#include <asn_application.h>

/* Including external dependencies */
#include "SPCSETKey.h"
#include "SPCTID.h"
#include "SPCSETKeylifetime.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SUPLAUTHRESP */
typedef struct SUPLAUTHRESP {
	SPCSETKey_t	 sPCSETKey;
	SPCTID_t	 sPCTID;
	SPCSETKeylifetime_t	*sPCSETKeylifetime;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SUPLAUTHRESP_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SUPLAUTHRESP;
extern asn_SEQUENCE_specifics_t asn_SPC_SUPLAUTHRESP_specs_1;
extern asn_TYPE_member_t asn_MBR_SUPLAUTHRESP_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _SUPLAUTHRESP_H_ */
#include <asn_internal.h>
