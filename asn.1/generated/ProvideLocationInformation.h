/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_ProvideLocationInformation_H_
#define	_ProvideLocationInformation_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ProvideLocationInformation-r9-IEs.h"
#include <NULL.h>
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ProvideLocationInformation__criticalExtensions_PR {
	ProvideLocationInformation__criticalExtensions_PR_NOTHING,	/* No components present */
	ProvideLocationInformation__criticalExtensions_PR_c1,
	ProvideLocationInformation__criticalExtensions_PR_criticalExtensionsFuture
} ProvideLocationInformation__criticalExtensions_PR;
typedef enum ProvideLocationInformation__criticalExtensions__c1_PR {
	ProvideLocationInformation__criticalExtensions__c1_PR_NOTHING,	/* No components present */
	ProvideLocationInformation__criticalExtensions__c1_PR_provideLocationInformation_r9,
	ProvideLocationInformation__criticalExtensions__c1_PR_spare3,
	ProvideLocationInformation__criticalExtensions__c1_PR_spare2,
	ProvideLocationInformation__criticalExtensions__c1_PR_spare1
} ProvideLocationInformation__criticalExtensions__c1_PR;

/* ProvideLocationInformation */
typedef struct ProvideLocationInformation {
	struct ProvideLocationInformation__criticalExtensions {
		ProvideLocationInformation__criticalExtensions_PR present;
		union ProvideLocationInformation__criticalExtensions_u {
			struct ProvideLocationInformation__criticalExtensions__c1 {
				ProvideLocationInformation__criticalExtensions__c1_PR present;
				union ProvideLocationInformation__criticalExtensions__c1_u {
					ProvideLocationInformation_r9_IEs_t	 provideLocationInformation_r9;
					NULL_t	 spare3;
					NULL_t	 spare2;
					NULL_t	 spare1;
				} choice;
				
				/* Context for parsing across buffer boundaries */
				asn_struct_ctx_t _asn_ctx;
			} c1;
			struct ProvideLocationInformation__criticalExtensions__criticalExtensionsFuture {
				
				/* Context for parsing across buffer boundaries */
				asn_struct_ctx_t _asn_ctx;
			} criticalExtensionsFuture;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} criticalExtensions;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ProvideLocationInformation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ProvideLocationInformation;
extern asn_SEQUENCE_specifics_t asn_SPC_ProvideLocationInformation_specs_1;
extern asn_TYPE_member_t asn_MBR_ProvideLocationInformation_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _ProvideLocationInformation_H_ */
#include <asn_internal.h>
