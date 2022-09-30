/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Ver2-ULP-Components"
 * 	found in "/home/martin/repos/LPP-Client/asn/ULP-Components.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_UTRAN_GANSSReferenceTime_H_
#define	_UTRAN_GANSSReferenceTime_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include "PrimaryCPICH-Info.h"
#include <constr_SEQUENCE.h>
#include "CellParametersID.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum UTRAN_GANSSReferenceTime__modeSpecificInfo_PR {
	UTRAN_GANSSReferenceTime__modeSpecificInfo_PR_NOTHING,	/* No components present */
	UTRAN_GANSSReferenceTime__modeSpecificInfo_PR_fdd,
	UTRAN_GANSSReferenceTime__modeSpecificInfo_PR_tdd
} UTRAN_GANSSReferenceTime__modeSpecificInfo_PR;

/* UTRAN-GANSSReferenceTime */
typedef struct UTRAN_GANSSReferenceTime {
	long	 ganssTOD;
	long	*utran_GANSSTimingOfCell	/* OPTIONAL */;
	struct UTRAN_GANSSReferenceTime__modeSpecificInfo {
		UTRAN_GANSSReferenceTime__modeSpecificInfo_PR present;
		union UTRAN_GANSSReferenceTime__modeSpecificInfo_u {
			struct UTRAN_GANSSReferenceTime__modeSpecificInfo__fdd {
				PrimaryCPICH_Info_t	 referenceIdentity;
				
				/* Context for parsing across buffer boundaries */
				asn_struct_ctx_t _asn_ctx;
			} fdd;
			struct UTRAN_GANSSReferenceTime__modeSpecificInfo__tdd {
				CellParametersID_t	 referenceIdentity;
				
				/* Context for parsing across buffer boundaries */
				asn_struct_ctx_t _asn_ctx;
			} tdd;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *modeSpecificInfo;
	long	 sfn;
	long	*ganss_TODUncertainty	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} UTRAN_GANSSReferenceTime_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UTRAN_GANSSReferenceTime;
extern asn_SEQUENCE_specifics_t asn_SPC_UTRAN_GANSSReferenceTime_specs_1;
extern asn_TYPE_member_t asn_MBR_UTRAN_GANSSReferenceTime_1[5];

#ifdef __cplusplus
}
#endif

#endif	/* _UTRAN_GANSSReferenceTime_H_ */
#include <asn_internal.h>
