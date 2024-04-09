/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#ifndef	_UTC_ModelSet4_H_
#define	_UTC_ModelSet4_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UTC-ModelSet4 */
typedef struct UTC_ModelSet4 {
	long	 utcA1wnt;
	long	 utcA0wnt;
	long	 utcTot;
	long	 utcWNt;
	long	 utcDeltaTls;
	long	 utcWNlsf;
	long	 utcDN;
	long	 utcDeltaTlsf;
	long	 utcStandardID;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} UTC_ModelSet4_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UTC_ModelSet4;
extern asn_SEQUENCE_specifics_t asn_SPC_UTC_ModelSet4_specs_1;
extern asn_TYPE_member_t asn_MBR_UTC_ModelSet4_1[9];

#ifdef __cplusplus
}
#endif

#endif	/* _UTC_ModelSet4_H_ */
#include <asn_internal.h>