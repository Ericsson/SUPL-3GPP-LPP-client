/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "NR-DL-TDOA-ProvideAssistanceData-r16.h"

#include "NR-DL-PRS-AssistanceData-r16.h"
#include "NR-SelectedDL-PRS-IndexList-r16.h"
#include "NR-PositionCalculationAssistance-r16.h"
#include "NR-DL-TDOA-Error-r16.h"
#include "NR-On-Demand-DL-PRS-Configurations-r17.h"
#include "NR-On-Demand-DL-PRS-Configurations-Selected-IndexList-r17.h"
#include "AreaID-CellList-r17.h"
static asn_TYPE_member_t asn_MBR_ext1_7[] = {
	{ ATF_POINTER, 3, offsetof(struct NR_DL_TDOA_ProvideAssistanceData_r16__ext1, nr_On_Demand_DL_PRS_Configurations_r17),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NR_On_Demand_DL_PRS_Configurations_r17,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			0
		},
		0, 0, /* No default value */
		"nr-On-Demand-DL-PRS-Configurations-r17"
		},
	{ ATF_POINTER, 2, offsetof(struct NR_DL_TDOA_ProvideAssistanceData_r16__ext1, nr_On_Demand_DL_PRS_Configurations_Selected_IndexList_r17),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NR_On_Demand_DL_PRS_Configurations_Selected_IndexList_r17,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			0
		},
		0, 0, /* No default value */
		"nr-On-Demand-DL-PRS-Configurations-Selected-IndexList-r17"
		},
	{ ATF_POINTER, 1, offsetof(struct NR_DL_TDOA_ProvideAssistanceData_r16__ext1, assistanceDataValidityArea_r17),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AreaID_CellList_r17,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			0
		},
		0, 0, /* No default value */
		"assistanceDataValidityArea-r17"
		},
};
static const int asn_MAP_ext1_oms_7[] = { 0, 1, 2 };
static const ber_tlv_tag_t asn_DEF_ext1_tags_7[] = {
	(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext1_tag2el_7[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* nr-On-Demand-DL-PRS-Configurations-r17 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* nr-On-Demand-DL-PRS-Configurations-Selected-IndexList-r17 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* assistanceDataValidityArea-r17 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext1_specs_7 = {
	sizeof(struct NR_DL_TDOA_ProvideAssistanceData_r16__ext1),
	offsetof(struct NR_DL_TDOA_ProvideAssistanceData_r16__ext1, _asn_ctx),
	asn_MAP_ext1_tag2el_7,
	3,	/* Count of tags in the map */
	asn_MAP_ext1_oms_7,	/* Optional members */
	3, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_ext1_7 = {
	"ext1",
	"ext1",
	&asn_OP_SEQUENCE,
	asn_DEF_ext1_tags_7,
	sizeof(asn_DEF_ext1_tags_7)
		/sizeof(asn_DEF_ext1_tags_7[0]) - 1, /* 1 */
	asn_DEF_ext1_tags_7,	/* Same as above */
	sizeof(asn_DEF_ext1_tags_7)
		/sizeof(asn_DEF_ext1_tags_7[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_ext1_7,
	3,	/* Elements count */
	&asn_SPC_ext1_specs_7	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_NR_DL_TDOA_ProvideAssistanceData_r16_1[] = {
	{ ATF_POINTER, 5, offsetof(struct NR_DL_TDOA_ProvideAssistanceData_r16, nr_DL_PRS_AssistanceData_r16),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NR_DL_PRS_AssistanceData_r16,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			0
		},
		0, 0, /* No default value */
		"nr-DL-PRS-AssistanceData-r16"
		},
	{ ATF_POINTER, 4, offsetof(struct NR_DL_TDOA_ProvideAssistanceData_r16, nr_SelectedDL_PRS_IndexList_r16),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NR_SelectedDL_PRS_IndexList_r16,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			0
		},
		0, 0, /* No default value */
		"nr-SelectedDL-PRS-IndexList-r16"
		},
	{ ATF_POINTER, 3, offsetof(struct NR_DL_TDOA_ProvideAssistanceData_r16, nr_PositionCalculationAssistance_r16),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NR_PositionCalculationAssistance_r16,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			0
		},
		0, 0, /* No default value */
		"nr-PositionCalculationAssistance-r16"
		},
	{ ATF_POINTER, 2, offsetof(struct NR_DL_TDOA_ProvideAssistanceData_r16, nr_DL_TDOA_Error_r16),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_NR_DL_TDOA_Error_r16,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			0
		},
		0, 0, /* No default value */
		"nr-DL-TDOA-Error-r16"
		},
	{ ATF_POINTER, 1, offsetof(struct NR_DL_TDOA_ProvideAssistanceData_r16, ext1),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		0,
		&asn_DEF_ext1_7,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			0
		},
		0, 0, /* No default value */
		"ext1"
		},
};
static const int asn_MAP_NR_DL_TDOA_ProvideAssistanceData_r16_oms_1[] = { 0, 1, 2, 3, 4 };
static const ber_tlv_tag_t asn_DEF_NR_DL_TDOA_ProvideAssistanceData_r16_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_NR_DL_TDOA_ProvideAssistanceData_r16_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* nr-DL-PRS-AssistanceData-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* nr-SelectedDL-PRS-IndexList-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* nr-PositionCalculationAssistance-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* nr-DL-TDOA-Error-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* ext1 */
};
asn_SEQUENCE_specifics_t asn_SPC_NR_DL_TDOA_ProvideAssistanceData_r16_specs_1 = {
	sizeof(struct NR_DL_TDOA_ProvideAssistanceData_r16),
	offsetof(struct NR_DL_TDOA_ProvideAssistanceData_r16, _asn_ctx),
	asn_MAP_NR_DL_TDOA_ProvideAssistanceData_r16_tag2el_1,
	5,	/* Count of tags in the map */
	asn_MAP_NR_DL_TDOA_ProvideAssistanceData_r16_oms_1,	/* Optional members */
	4, 1,	/* Root/Additions */
	4,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_NR_DL_TDOA_ProvideAssistanceData_r16 = {
	"NR-DL-TDOA-ProvideAssistanceData-r16",
	"NR-DL-TDOA-ProvideAssistanceData-r16",
	&asn_OP_SEQUENCE,
	asn_DEF_NR_DL_TDOA_ProvideAssistanceData_r16_tags_1,
	sizeof(asn_DEF_NR_DL_TDOA_ProvideAssistanceData_r16_tags_1)
		/sizeof(asn_DEF_NR_DL_TDOA_ProvideAssistanceData_r16_tags_1[0]), /* 1 */
	asn_DEF_NR_DL_TDOA_ProvideAssistanceData_r16_tags_1,	/* Same as above */
	sizeof(asn_DEF_NR_DL_TDOA_ProvideAssistanceData_r16_tags_1)
		/sizeof(asn_DEF_NR_DL_TDOA_ProvideAssistanceData_r16_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_NR_DL_TDOA_ProvideAssistanceData_r16_1,
	5,	/* Elements count */
	&asn_SPC_NR_DL_TDOA_ProvideAssistanceData_r16_specs_1	/* Additional specs */
};

