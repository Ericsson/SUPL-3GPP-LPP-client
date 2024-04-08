/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_NR_PositionCalculationAssistance_r16_H_
#define	_NR_PositionCalculationAssistance_r16_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct NR_TRP_LocationInfo_r16;
struct NR_DL_PRS_BeamInfo_r16;
struct NR_RTD_Info_r16;
struct NR_TRP_BeamAntennaInfo_r17;
struct NR_DL_PRS_ExpectedLOS_NLOS_Assistance_r17;
struct NR_DL_PRS_TRP_TEG_Info_r17;

/* NR-PositionCalculationAssistance-r16 */
typedef struct NR_PositionCalculationAssistance_r16 {
	struct NR_TRP_LocationInfo_r16	*nr_TRP_LocationInfo_r16;	/* OPTIONAL */
	struct NR_DL_PRS_BeamInfo_r16	*nr_DL_PRS_BeamInfo_r16;	/* OPTIONAL */
	struct NR_RTD_Info_r16	*nr_RTD_Info_r16;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct NR_PositionCalculationAssistance_r16__ext1 {
		struct NR_TRP_BeamAntennaInfo_r17	*nr_TRP_BeamAntennaInfo_r17;	/* OPTIONAL */
		struct NR_DL_PRS_ExpectedLOS_NLOS_Assistance_r17	*nr_DL_PRS_Expected_LOS_NLOS_Assistance_r17;	/* OPTIONAL */
		struct NR_DL_PRS_TRP_TEG_Info_r17	*nr_DL_PRS_TRP_TEG_Info_r17;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext1;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NR_PositionCalculationAssistance_r16_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NR_PositionCalculationAssistance_r16;
extern asn_SEQUENCE_specifics_t asn_SPC_NR_PositionCalculationAssistance_r16_specs_1;
extern asn_TYPE_member_t asn_MBR_NR_PositionCalculationAssistance_r16_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _NR_PositionCalculationAssistance_r16_H_ */
#include <asn_internal.h>
