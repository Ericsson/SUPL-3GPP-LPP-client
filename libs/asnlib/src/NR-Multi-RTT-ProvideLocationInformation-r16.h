/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "/home/martin/repos/LPP-Client/asn/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_NR_Multi_RTT_ProvideLocationInformation_r16_H_
#define	_NR_Multi_RTT_ProvideLocationInformation_r16_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct NR_Multi_RTT_SignalMeasurementInformation_r16;
struct NR_Multi_RTT_Error_r16;

/* NR-Multi-RTT-ProvideLocationInformation-r16 */
typedef struct NR_Multi_RTT_ProvideLocationInformation_r16 {
	struct NR_Multi_RTT_SignalMeasurementInformation_r16	*nr_Multi_RTT_SignalMeasurementInformation_r16	/* OPTIONAL */;
	struct NR_Multi_RTT_Error_r16	*nr_Multi_RTT_Error_r16	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NR_Multi_RTT_ProvideLocationInformation_r16_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NR_Multi_RTT_ProvideLocationInformation_r16;
extern asn_SEQUENCE_specifics_t asn_SPC_NR_Multi_RTT_ProvideLocationInformation_r16_specs_1;
extern asn_TYPE_member_t asn_MBR_NR_Multi_RTT_ProvideLocationInformation_r16_1[2];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "NR-Multi-RTT-SignalMeasurementInformation-r16.h"
#include "NR-Multi-RTT-Error-r16.h"

#endif	/* _NR_Multi_RTT_ProvideLocationInformation_r16_H_ */
#include <asn_internal.h>
