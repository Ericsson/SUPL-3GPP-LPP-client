/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_SRS_CapabilityPerBand_r16_H_
#define	_SRS_CapabilityPerBand_r16_H_


#include <asn_application.h>

/* Including external dependencies */
#include "FreqBandIndicatorNR-r16.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct OLPC_SRS_Pos_r16;
struct SpatialRelationsSRS_Pos_r16;
struct PosSRS_RRC_Inactive_InInitialUL_BWP_r17;
struct PosSRS_RRC_Inactive_OutsideInitialUL_BWP_r17;
struct PosSRS_SP_RRC_Inactive_InInitialUL_BWP_r17;

/* SRS-CapabilityPerBand-r16 */
typedef struct SRS_CapabilityPerBand_r16 {
	FreqBandIndicatorNR_r16_t	 freqBandIndicatorNR_r16;
	struct OLPC_SRS_Pos_r16	*olpc_SRS_Pos_r16;	/* OPTIONAL */
	struct SpatialRelationsSRS_Pos_r16	*spatialRelationsSRS_Pos_r16;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct SRS_CapabilityPerBand_r16__ext1 {
		struct PosSRS_RRC_Inactive_InInitialUL_BWP_r17	*posSRS_RRC_Inactive_InInitialUL_BWP_r17;	/* OPTIONAL */
		struct PosSRS_RRC_Inactive_OutsideInitialUL_BWP_r17	*posSRS_RRC_Inactive_OutsideInitialUL_BWP_r17;	/* OPTIONAL */
		struct OLPC_SRS_Pos_r16	*olpc_SRS_PosRRC_Inactive_r17;	/* OPTIONAL */
		struct SpatialRelationsSRS_Pos_r16	*spatialRelationsSRS_PosRRC_Inactive_r17;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext1;
	struct SRS_CapabilityPerBand_r16__ext2 {
		struct PosSRS_SP_RRC_Inactive_InInitialUL_BWP_r17	*posSRS_SP_RRC_Inactive_InInitialUL_BWP_r17;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext2;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SRS_CapabilityPerBand_r16_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SRS_CapabilityPerBand_r16;
extern asn_SEQUENCE_specifics_t asn_SPC_SRS_CapabilityPerBand_r16_specs_1;
extern asn_TYPE_member_t asn_MBR_SRS_CapabilityPerBand_r16_1[5];

#ifdef __cplusplus
}
#endif

#endif	/* _SRS_CapabilityPerBand_r16_H_ */
#include <asn_internal.h>
