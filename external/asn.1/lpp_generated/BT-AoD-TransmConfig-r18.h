/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#ifndef	_BT_AoD_TransmConfig_r18_H_
#define	_BT_AoD_TransmConfig_r18_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <NativeEnumerated.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum BT_AoD_TransmConfig_r18__cte_Type2us_r18 {
	BT_AoD_TransmConfig_r18__cte_Type2us_r18_true	= 0
} e_BT_AoD_TransmConfig_r18__cte_Type2us_r18;
typedef enum BT_AoD_TransmConfig_r18__tx_PHY_M2_r18 {
	BT_AoD_TransmConfig_r18__tx_PHY_M2_r18_true	= 0
} e_BT_AoD_TransmConfig_r18__tx_PHY_M2_r18;

/* BT-AoD-TransmConfig-r18 */
typedef struct BT_AoD_TransmConfig_r18 {
	long	 primaryAdvInterval_r18;
	long	 secondAdvInterval_r18;
	long	 cte_Length_r18;
	long	 cte_Count_r18;
	long	*cte_Type2us_r18;	/* OPTIONAL */
	long	*tx_PHY_M2_r18;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} BT_AoD_TransmConfig_r18_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_cte_Type2us_r18_6;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_tx_PHY_M2_r18_8;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_BT_AoD_TransmConfig_r18;
extern asn_SEQUENCE_specifics_t asn_SPC_BT_AoD_TransmConfig_r18_specs_1;
extern asn_TYPE_member_t asn_MBR_BT_AoD_TransmConfig_r18_1[6];

#ifdef __cplusplus
}
#endif

#endif	/* _BT_AoD_TransmConfig_r18_H_ */
#include <asn_internal.h>
