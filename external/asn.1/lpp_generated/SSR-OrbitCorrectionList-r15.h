/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#ifndef	_SSR_OrbitCorrectionList_r15_H_
#define	_SSR_OrbitCorrectionList_r15_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct SSR_OrbitCorrectionSatelliteElement_r15;

/* SSR-OrbitCorrectionList-r15 */
typedef struct SSR_OrbitCorrectionList_r15 {
	A_SEQUENCE_OF(struct SSR_OrbitCorrectionSatelliteElement_r15) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SSR_OrbitCorrectionList_r15_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SSR_OrbitCorrectionList_r15;
extern asn_SET_OF_specifics_t asn_SPC_SSR_OrbitCorrectionList_r15_specs_1;
extern asn_TYPE_member_t asn_MBR_SSR_OrbitCorrectionList_r15_1[1];
extern asn_per_constraints_t asn_PER_type_SSR_OrbitCorrectionList_r15_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _SSR_OrbitCorrectionList_r15_H_ */
#include <asn_internal.h>