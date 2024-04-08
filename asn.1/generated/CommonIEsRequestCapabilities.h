/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_CommonIEsRequestCapabilities_H_
#define	_CommonIEsRequestCapabilities_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CommonIEsRequestCapabilities__ext1__lpp_message_segmentation_req_r14 {
	CommonIEsRequestCapabilities__ext1__lpp_message_segmentation_req_r14_serverToTarget	= 0,
	CommonIEsRequestCapabilities__ext1__lpp_message_segmentation_req_r14_targetToServer	= 1
} e_CommonIEsRequestCapabilities__ext1__lpp_message_segmentation_req_r14;

/* CommonIEsRequestCapabilities */
typedef struct CommonIEsRequestCapabilities {
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct CommonIEsRequestCapabilities__ext1 {
		BIT_STRING_t	*lpp_message_segmentation_req_r14;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext1;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CommonIEsRequestCapabilities_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CommonIEsRequestCapabilities;
extern asn_SEQUENCE_specifics_t asn_SPC_CommonIEsRequestCapabilities_specs_1;
extern asn_TYPE_member_t asn_MBR_CommonIEsRequestCapabilities_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _CommonIEsRequestCapabilities_H_ */
#include <asn_internal.h>
