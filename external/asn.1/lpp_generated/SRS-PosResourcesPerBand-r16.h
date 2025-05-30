/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#ifndef	_SRS_PosResourcesPerBand_r16_H_
#define	_SRS_PosResourcesPerBand_r16_H_


#include <asn_application.h>

/* Including external dependencies */
#include "FreqBandIndicatorNR-r16.h"
#include <NativeEnumerated.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourceSetsPerBWP_r16 {
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourceSetsPerBWP_r16_n1	= 0,
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourceSetsPerBWP_r16_n2	= 1,
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourceSetsPerBWP_r16_n4	= 2,
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourceSetsPerBWP_r16_n8	= 3,
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourceSetsPerBWP_r16_n12	= 4,
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourceSetsPerBWP_r16_n16	= 5
} e_SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourceSetsPerBWP_r16;
typedef enum SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourcesPerBWP_r16 {
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourcesPerBWP_r16_n1	= 0,
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourcesPerBWP_r16_n2	= 1,
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourcesPerBWP_r16_n4	= 2,
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourcesPerBWP_r16_n8	= 3,
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourcesPerBWP_r16_n16	= 4,
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourcesPerBWP_r16_n32	= 5,
	SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourcesPerBWP_r16_n64	= 6
} e_SRS_PosResourcesPerBand_r16__maxNumberSRS_PosResourcesPerBWP_r16;
typedef enum SRS_PosResourcesPerBand_r16__maxNumberPeriodicSRS_PosResourcesPerBWP_r16 {
	SRS_PosResourcesPerBand_r16__maxNumberPeriodicSRS_PosResourcesPerBWP_r16_n1	= 0,
	SRS_PosResourcesPerBand_r16__maxNumberPeriodicSRS_PosResourcesPerBWP_r16_n2	= 1,
	SRS_PosResourcesPerBand_r16__maxNumberPeriodicSRS_PosResourcesPerBWP_r16_n4	= 2,
	SRS_PosResourcesPerBand_r16__maxNumberPeriodicSRS_PosResourcesPerBWP_r16_n8	= 3,
	SRS_PosResourcesPerBand_r16__maxNumberPeriodicSRS_PosResourcesPerBWP_r16_n16	= 4,
	SRS_PosResourcesPerBand_r16__maxNumberPeriodicSRS_PosResourcesPerBWP_r16_n32	= 5,
	SRS_PosResourcesPerBand_r16__maxNumberPeriodicSRS_PosResourcesPerBWP_r16_n64	= 6
} e_SRS_PosResourcesPerBand_r16__maxNumberPeriodicSRS_PosResourcesPerBWP_r16;
typedef enum SRS_PosResourcesPerBand_r16__maxNumberAP_SRS_PosResourcesPerBWP_r16 {
	SRS_PosResourcesPerBand_r16__maxNumberAP_SRS_PosResourcesPerBWP_r16_n1	= 0,
	SRS_PosResourcesPerBand_r16__maxNumberAP_SRS_PosResourcesPerBWP_r16_n2	= 1,
	SRS_PosResourcesPerBand_r16__maxNumberAP_SRS_PosResourcesPerBWP_r16_n4	= 2,
	SRS_PosResourcesPerBand_r16__maxNumberAP_SRS_PosResourcesPerBWP_r16_n8	= 3,
	SRS_PosResourcesPerBand_r16__maxNumberAP_SRS_PosResourcesPerBWP_r16_n16	= 4,
	SRS_PosResourcesPerBand_r16__maxNumberAP_SRS_PosResourcesPerBWP_r16_n32	= 5,
	SRS_PosResourcesPerBand_r16__maxNumberAP_SRS_PosResourcesPerBWP_r16_n64	= 6
} e_SRS_PosResourcesPerBand_r16__maxNumberAP_SRS_PosResourcesPerBWP_r16;
typedef enum SRS_PosResourcesPerBand_r16__maxNumberSP_SRS_PosResourcesPerBWP_r16 {
	SRS_PosResourcesPerBand_r16__maxNumberSP_SRS_PosResourcesPerBWP_r16_n1	= 0,
	SRS_PosResourcesPerBand_r16__maxNumberSP_SRS_PosResourcesPerBWP_r16_n2	= 1,
	SRS_PosResourcesPerBand_r16__maxNumberSP_SRS_PosResourcesPerBWP_r16_n4	= 2,
	SRS_PosResourcesPerBand_r16__maxNumberSP_SRS_PosResourcesPerBWP_r16_n8	= 3,
	SRS_PosResourcesPerBand_r16__maxNumberSP_SRS_PosResourcesPerBWP_r16_n16	= 4,
	SRS_PosResourcesPerBand_r16__maxNumberSP_SRS_PosResourcesPerBWP_r16_n32	= 5,
	SRS_PosResourcesPerBand_r16__maxNumberSP_SRS_PosResourcesPerBWP_r16_n64	= 6
} e_SRS_PosResourcesPerBand_r16__maxNumberSP_SRS_PosResourcesPerBWP_r16;

/* Forward declarations */
struct PosSRS_BWA_RRC_Connected_r18;
struct PosSRS_BWA_IndependentCA_RRC_Connected_r18;

/* SRS-PosResourcesPerBand-r16 */
typedef struct SRS_PosResourcesPerBand_r16 {
	FreqBandIndicatorNR_r16_t	 freqBandIndicatorNR_r16;
	long	 maxNumberSRS_PosResourceSetsPerBWP_r16;
	long	 maxNumberSRS_PosResourcesPerBWP_r16;
	long	 maxNumberPeriodicSRS_PosResourcesPerBWP_r16;
	long	*maxNumberAP_SRS_PosResourcesPerBWP_r16;	/* OPTIONAL */
	long	*maxNumberSP_SRS_PosResourcesPerBWP_r16;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct SRS_PosResourcesPerBand_r16__ext1 {
		struct PosSRS_BWA_RRC_Connected_r18	*posSRS_BWA_RRC_Connected_r18;	/* OPTIONAL */
		struct PosSRS_BWA_IndependentCA_RRC_Connected_r18	*posSRS_BWA_IndependentCA_RRC_Connected_r18;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext1;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SRS_PosResourcesPerBand_r16_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_maxNumberSRS_PosResourceSetsPerBWP_r16_3;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_maxNumberSRS_PosResourcesPerBWP_r16_10;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_maxNumberPeriodicSRS_PosResourcesPerBWP_r16_18;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_maxNumberAP_SRS_PosResourcesPerBWP_r16_26;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_maxNumberSP_SRS_PosResourcesPerBWP_r16_34;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_SRS_PosResourcesPerBand_r16;
extern asn_SEQUENCE_specifics_t asn_SPC_SRS_PosResourcesPerBand_r16_specs_1;
extern asn_TYPE_member_t asn_MBR_SRS_PosResourcesPerBand_r16_1[7];

#ifdef __cplusplus
}
#endif

#endif	/* _SRS_PosResourcesPerBand_r16_H_ */
#include <asn_internal.h>
