/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_NR_DL_PRS_ExpectedLOS_NLOS_Assistance_r17_H_
#define	_NR_DL_PRS_ExpectedLOS_NLOS_Assistance_r17_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerFreqLayer_r17;

/* NR-DL-PRS-ExpectedLOS-NLOS-Assistance-r17 */
typedef struct NR_DL_PRS_ExpectedLOS_NLOS_Assistance_r17 {
	A_SEQUENCE_OF(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerFreqLayer_r17) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NR_DL_PRS_ExpectedLOS_NLOS_Assistance_r17_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NR_DL_PRS_ExpectedLOS_NLOS_Assistance_r17;
extern asn_SET_OF_specifics_t asn_SPC_NR_DL_PRS_ExpectedLOS_NLOS_Assistance_r17_specs_1;
extern asn_TYPE_member_t asn_MBR_NR_DL_PRS_ExpectedLOS_NLOS_Assistance_r17_1[1];
extern asn_per_constraints_t asn_PER_type_NR_DL_PRS_ExpectedLOS_NLOS_Assistance_r17_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _NR_DL_PRS_ExpectedLOS_NLOS_Assistance_r17_H_ */
#include <asn_internal.h>
