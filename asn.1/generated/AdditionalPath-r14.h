/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "/home/martin/repos/LPP-Client/asn/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_AdditionalPath_r14_H_
#define	_AdditionalPath_r14_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct OTDOA_MeasQuality;

/* AdditionalPath-r14 */
typedef struct AdditionalPath_r14 {
	long	 relativeTimeDifference_r14;
	struct OTDOA_MeasQuality	*path_Quality_r14	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AdditionalPath_r14_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AdditionalPath_r14;
extern asn_SEQUENCE_specifics_t asn_SPC_AdditionalPath_r14_specs_1;
extern asn_TYPE_member_t asn_MBR_AdditionalPath_r14_1[2];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "OTDOA-MeasQuality.h"

#endif	/* _AdditionalPath_r14_H_ */
#include <asn_internal.h>