/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "/home/martin/repos/LPP-Client/asn/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_NavIC_CorrectionListAutoNav_r16_H_
#define	_NavIC_CorrectionListAutoNav_r16_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct NavIC_CorrectionElementAutoNav_r16;

/* NavIC-CorrectionListAutoNav-r16 */
typedef struct NavIC_CorrectionListAutoNav_r16 {
	A_SEQUENCE_OF(struct NavIC_CorrectionElementAutoNav_r16) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NavIC_CorrectionListAutoNav_r16_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NavIC_CorrectionListAutoNav_r16;
extern asn_SET_OF_specifics_t asn_SPC_NavIC_CorrectionListAutoNav_r16_specs_1;
extern asn_TYPE_member_t asn_MBR_NavIC_CorrectionListAutoNav_r16_1[1];
extern asn_per_constraints_t asn_PER_type_NavIC_CorrectionListAutoNav_r16_constr_1;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "NavIC-CorrectionElementAutoNav-r16.h"

#endif	/* _NavIC_CorrectionListAutoNav_r16_H_ */
#include <asn_internal.h>