/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "/home/martin/repos/LPP-Client/asn/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_DeltaTime_r15_H_
#define	_DeltaTime_r15_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum DeltaTime_r15_PR {
	DeltaTime_r15_PR_NOTHING,	/* No components present */
	DeltaTime_r15_PR_deltaTimeSec_r15,
	DeltaTime_r15_PR_deltaTimeSFN_r15
	/* Extensions may appear below */
	
} DeltaTime_r15_PR;

/* DeltaTime-r15 */
typedef struct DeltaTime_r15 {
	DeltaTime_r15_PR present;
	union DeltaTime_r15_u {
		long	 deltaTimeSec_r15;
		long	 deltaTimeSFN_r15;
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DeltaTime_r15_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DeltaTime_r15;
extern asn_CHOICE_specifics_t asn_SPC_DeltaTime_r15_specs_1;
extern asn_TYPE_member_t asn_MBR_DeltaTime_r15_1[2];
extern asn_per_constraints_t asn_PER_type_DeltaTime_r15_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _DeltaTime_r15_H_ */
#include <asn_internal.h>
