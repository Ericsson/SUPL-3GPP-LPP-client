/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_NR_DL_TDOA_Error_r16_H_
#define	_NR_DL_TDOA_Error_r16_H_


#include <asn_application.h>

/* Including external dependencies */
#include "NR-DL-TDOA-LocationServerErrorCauses-r16.h"
#include "NR-DL-TDOA-TargetDeviceErrorCauses-r16.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum NR_DL_TDOA_Error_r16_PR {
	NR_DL_TDOA_Error_r16_PR_NOTHING,	/* No components present */
	NR_DL_TDOA_Error_r16_PR_locationServerErrorCauses_r16,
	NR_DL_TDOA_Error_r16_PR_targetDeviceErrorCauses_r16
	/* Extensions may appear below */
	
} NR_DL_TDOA_Error_r16_PR;

/* NR-DL-TDOA-Error-r16 */
typedef struct NR_DL_TDOA_Error_r16 {
	NR_DL_TDOA_Error_r16_PR present;
	union NR_DL_TDOA_Error_r16_u {
		NR_DL_TDOA_LocationServerErrorCauses_r16_t	 locationServerErrorCauses_r16;
		NR_DL_TDOA_TargetDeviceErrorCauses_r16_t	 targetDeviceErrorCauses_r16;
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NR_DL_TDOA_Error_r16_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NR_DL_TDOA_Error_r16;
extern asn_CHOICE_specifics_t asn_SPC_NR_DL_TDOA_Error_r16_specs_1;
extern asn_TYPE_member_t asn_MBR_NR_DL_TDOA_Error_r16_1[2];
extern asn_per_constraints_t asn_PER_type_NR_DL_TDOA_Error_r16_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _NR_DL_TDOA_Error_r16_H_ */
#include <asn_internal.h>
