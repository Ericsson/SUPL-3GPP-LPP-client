/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_GridElement_r16_H_
#define	_GridElement_r16_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct TropospericDelayCorrection_r16;
struct STEC_ResidualSatList_r16;

/* GridElement-r16 */
typedef struct GridElement_r16 {
	struct TropospericDelayCorrection_r16	*tropospericDelayCorrection_r16;	/* OPTIONAL */
	struct STEC_ResidualSatList_r16	*stec_ResidualSatList_r16;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GridElement_r16_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GridElement_r16;
extern asn_SEQUENCE_specifics_t asn_SPC_GridElement_r16_specs_1;
extern asn_TYPE_member_t asn_MBR_GridElement_r16_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _GridElement_r16_H_ */
#include <asn_internal.h>
