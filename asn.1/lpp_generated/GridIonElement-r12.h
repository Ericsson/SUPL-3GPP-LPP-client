/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#ifndef	_GridIonElement_r12_H_
#define	_GridIonElement_r12_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GridIonElement-r12 */
typedef struct GridIonElement_r12 {
	long	 igp_ID_r12;
	long	 dt_r12;
	long	 givei_r12;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GridIonElement_r12_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GridIonElement_r12;
extern asn_SEQUENCE_specifics_t asn_SPC_GridIonElement_r12_specs_1;
extern asn_TYPE_member_t asn_MBR_GridIonElement_r12_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _GridIonElement_r12_H_ */
#include <asn_internal.h>