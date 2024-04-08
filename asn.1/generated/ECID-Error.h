/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_ECID_Error_H_
#define	_ECID_Error_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ECID-LocationServerErrorCauses.h"
#include "ECID-TargetDeviceErrorCauses.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ECID_Error_PR {
	ECID_Error_PR_NOTHING,	/* No components present */
	ECID_Error_PR_locationServerErrorCauses,
	ECID_Error_PR_targetDeviceErrorCauses
	/* Extensions may appear below */
	
} ECID_Error_PR;

/* ECID-Error */
typedef struct ECID_Error {
	ECID_Error_PR present;
	union ECID_Error_u {
		ECID_LocationServerErrorCauses_t	 locationServerErrorCauses;
		ECID_TargetDeviceErrorCauses_t	 targetDeviceErrorCauses;
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ECID_Error_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ECID_Error;
extern asn_CHOICE_specifics_t asn_SPC_ECID_Error_specs_1;
extern asn_TYPE_member_t asn_MBR_ECID_Error_1[2];
extern asn_per_constraints_t asn_PER_type_ECID_Error_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _ECID_Error_H_ */
#include <asn_internal.h>
