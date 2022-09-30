/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "/home/martin/repos/LPP-Client/asn/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_CommonIEsProvideCapabilities_H_
#define	_CommonIEsProvideCapabilities_H_


#include <asn_application.h>

/* Including external dependencies */
#include "SegmentationInfo-r14.h"
#include <BIT_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CommonIEsProvideCapabilities__ext1__lpp_message_segmentation_r14 {
	CommonIEsProvideCapabilities__ext1__lpp_message_segmentation_r14_serverToTarget	= 0,
	CommonIEsProvideCapabilities__ext1__lpp_message_segmentation_r14_targetToServer	= 1
} e_CommonIEsProvideCapabilities__ext1__lpp_message_segmentation_r14;

/* CommonIEsProvideCapabilities */
typedef struct CommonIEsProvideCapabilities {
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct CommonIEsProvideCapabilities__ext1 {
		SegmentationInfo_r14_t	*segmentationInfo_r14	/* OPTIONAL */;
		BIT_STRING_t	*lpp_message_segmentation_r14	/* OPTIONAL */;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext1;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CommonIEsProvideCapabilities_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CommonIEsProvideCapabilities;
extern asn_SEQUENCE_specifics_t asn_SPC_CommonIEsProvideCapabilities_specs_1;
extern asn_TYPE_member_t asn_MBR_CommonIEsProvideCapabilities_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _CommonIEsProvideCapabilities_H_ */
#include <asn_internal.h>
