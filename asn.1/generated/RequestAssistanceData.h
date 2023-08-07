/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "/home/martin/repos/LPP-Client/asn/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_RequestAssistanceData_H_
#define	_RequestAssistanceData_H_


#include <asn_application.h>

/* Including external dependencies */
#include "RequestAssistanceData-r9-IEs.h"
#include <NULL.h>
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum RequestAssistanceData__criticalExtensions_PR {
	RequestAssistanceData__criticalExtensions_PR_NOTHING,	/* No components present */
	RequestAssistanceData__criticalExtensions_PR_c1,
	RequestAssistanceData__criticalExtensions_PR_criticalExtensionsFuture
} RequestAssistanceData__criticalExtensions_PR;
typedef enum RequestAssistanceData__criticalExtensions__c1_PR {
	RequestAssistanceData__criticalExtensions__c1_PR_NOTHING,	/* No components present */
	RequestAssistanceData__criticalExtensions__c1_PR_requestAssistanceData_r9,
	RequestAssistanceData__criticalExtensions__c1_PR_spare3,
	RequestAssistanceData__criticalExtensions__c1_PR_spare2,
	RequestAssistanceData__criticalExtensions__c1_PR_spare1
} RequestAssistanceData__criticalExtensions__c1_PR;

/* RequestAssistanceData */
typedef struct RequestAssistanceData {
	struct RequestAssistanceData__criticalExtensions {
		RequestAssistanceData__criticalExtensions_PR present;
		union RequestAssistanceData__criticalExtensions_u {
			struct RequestAssistanceData__criticalExtensions__c1 {
				RequestAssistanceData__criticalExtensions__c1_PR present;
				union RequestAssistanceData__criticalExtensions__c1_u {
					RequestAssistanceData_r9_IEs_t	 requestAssistanceData_r9;
					NULL_t	 spare3;
					NULL_t	 spare2;
					NULL_t	 spare1;
				} choice;
				
				/* Context for parsing across buffer boundaries */
				asn_struct_ctx_t _asn_ctx;
			} c1;
			struct RequestAssistanceData__criticalExtensions__criticalExtensionsFuture {
				
				/* Context for parsing across buffer boundaries */
				asn_struct_ctx_t _asn_ctx;
			} criticalExtensionsFuture;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} criticalExtensions;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RequestAssistanceData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RequestAssistanceData;
extern asn_SEQUENCE_specifics_t asn_SPC_RequestAssistanceData_specs_1;
extern asn_TYPE_member_t asn_MBR_RequestAssistanceData_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _RequestAssistanceData_H_ */
#include <asn_internal.h>
