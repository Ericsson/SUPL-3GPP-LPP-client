/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#ifndef	_NR_Multi_RTT_MeasurementCapability_r16_H_
#define	_NR_Multi_RTT_MeasurementCapability_r16_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <NativeEnumerated.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum NR_Multi_RTT_MeasurementCapability_r16__supportOfRSRP_MeasFR1_r16 {
	NR_Multi_RTT_MeasurementCapability_r16__supportOfRSRP_MeasFR1_r16_supported	= 0
} e_NR_Multi_RTT_MeasurementCapability_r16__supportOfRSRP_MeasFR1_r16;
typedef enum NR_Multi_RTT_MeasurementCapability_r16__supportOfRSRP_MeasFR2_r16 {
	NR_Multi_RTT_MeasurementCapability_r16__supportOfRSRP_MeasFR2_r16_supported	= 0
} e_NR_Multi_RTT_MeasurementCapability_r16__supportOfRSRP_MeasFR2_r16;
typedef enum NR_Multi_RTT_MeasurementCapability_r16__srs_AssocPRS_MultiLayersFR1_r16 {
	NR_Multi_RTT_MeasurementCapability_r16__srs_AssocPRS_MultiLayersFR1_r16_supported	= 0
} e_NR_Multi_RTT_MeasurementCapability_r16__srs_AssocPRS_MultiLayersFR1_r16;
typedef enum NR_Multi_RTT_MeasurementCapability_r16__srs_AssocPRS_MultiLayersFR2_r16 {
	NR_Multi_RTT_MeasurementCapability_r16__srs_AssocPRS_MultiLayersFR2_r16_supported	= 0
} e_NR_Multi_RTT_MeasurementCapability_r16__srs_AssocPRS_MultiLayersFR2_r16;

/* Forward declarations */
struct NR_UE_TEG_Capability_r17;
struct Multi_RTT_MeasCapabilityPerBand_r17;

/* NR-Multi-RTT-MeasurementCapability-r16 */
typedef struct NR_Multi_RTT_MeasurementCapability_r16 {
	long	*maxNrOfRx_TX_MeasFR1_r16;	/* OPTIONAL */
	long	*maxNrOfRx_TX_MeasFR2_r16;	/* OPTIONAL */
	long	*supportOfRSRP_MeasFR1_r16;	/* OPTIONAL */
	long	*supportOfRSRP_MeasFR2_r16;	/* OPTIONAL */
	long	*srs_AssocPRS_MultiLayersFR1_r16;	/* OPTIONAL */
	long	*srs_AssocPRS_MultiLayersFR2_r16;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct NR_Multi_RTT_MeasurementCapability_r16__ext1 {
		struct NR_UE_TEG_Capability_r17	*nr_UE_TEG_Capability_r17;	/* OPTIONAL */
		struct NR_Multi_RTT_MeasurementCapability_r16__ext1__multi_RTT_MeasCapabilityBandList_r17 {
			A_SEQUENCE_OF(struct Multi_RTT_MeasCapabilityPerBand_r17) list;
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} *multi_RTT_MeasCapabilityBandList_r17;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext1;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NR_Multi_RTT_MeasurementCapability_r16_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_supportOfRSRP_MeasFR1_r16_4;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_supportOfRSRP_MeasFR2_r16_6;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_srs_AssocPRS_MultiLayersFR1_r16_8;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_srs_AssocPRS_MultiLayersFR2_r16_10;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_NR_Multi_RTT_MeasurementCapability_r16;
extern asn_SEQUENCE_specifics_t asn_SPC_NR_Multi_RTT_MeasurementCapability_r16_specs_1;
extern asn_TYPE_member_t asn_MBR_NR_Multi_RTT_MeasurementCapability_r16_1[7];

#ifdef __cplusplus
}
#endif

#endif	/* _NR_Multi_RTT_MeasurementCapability_r16_H_ */
#include <asn_internal.h>