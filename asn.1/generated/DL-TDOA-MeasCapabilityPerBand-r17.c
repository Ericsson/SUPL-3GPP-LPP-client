/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "DL-TDOA-MeasCapabilityPerBand-r17.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_supportOfDL_PRS_FirstPathRSRP_r17_constr_3 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 0,  0,  0,  0 }	/* (0..0) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_dl_PRS_MeasRRC_Inactive_r17_constr_5 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 0,  0,  0,  0 }	/* (0..0) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static const asn_INTEGER_enum_map_t asn_MAP_supportOfDL_PRS_FirstPathRSRP_r17_value2enum_3[] = {
	{ 0,	9,	"supported" }
};
static const unsigned int asn_MAP_supportOfDL_PRS_FirstPathRSRP_r17_enum2value_3[] = {
	0	/* supported(0) */
};
static const asn_INTEGER_specifics_t asn_SPC_supportOfDL_PRS_FirstPathRSRP_r17_specs_3 = {
	asn_MAP_supportOfDL_PRS_FirstPathRSRP_r17_value2enum_3,	/* "tag" => N; sorted by tag */
	asn_MAP_supportOfDL_PRS_FirstPathRSRP_r17_enum2value_3,	/* N => "tag"; sorted by N */
	1,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_supportOfDL_PRS_FirstPathRSRP_r17_tags_3[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_supportOfDL_PRS_FirstPathRSRP_r17_3 = {
	"supportOfDL-PRS-FirstPathRSRP-r17",
	"supportOfDL-PRS-FirstPathRSRP-r17",
	&asn_OP_NativeEnumerated,
	asn_DEF_supportOfDL_PRS_FirstPathRSRP_r17_tags_3,
	sizeof(asn_DEF_supportOfDL_PRS_FirstPathRSRP_r17_tags_3)
		/sizeof(asn_DEF_supportOfDL_PRS_FirstPathRSRP_r17_tags_3[0]) - 1, /* 1 */
	asn_DEF_supportOfDL_PRS_FirstPathRSRP_r17_tags_3,	/* Same as above */
	sizeof(asn_DEF_supportOfDL_PRS_FirstPathRSRP_r17_tags_3)
		/sizeof(asn_DEF_supportOfDL_PRS_FirstPathRSRP_r17_tags_3[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_supportOfDL_PRS_FirstPathRSRP_r17_constr_3,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_supportOfDL_PRS_FirstPathRSRP_r17_specs_3	/* Additional specs */
};

static const asn_INTEGER_enum_map_t asn_MAP_dl_PRS_MeasRRC_Inactive_r17_value2enum_5[] = {
	{ 0,	9,	"supported" }
};
static const unsigned int asn_MAP_dl_PRS_MeasRRC_Inactive_r17_enum2value_5[] = {
	0	/* supported(0) */
};
static const asn_INTEGER_specifics_t asn_SPC_dl_PRS_MeasRRC_Inactive_r17_specs_5 = {
	asn_MAP_dl_PRS_MeasRRC_Inactive_r17_value2enum_5,	/* "tag" => N; sorted by tag */
	asn_MAP_dl_PRS_MeasRRC_Inactive_r17_enum2value_5,	/* N => "tag"; sorted by N */
	1,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_dl_PRS_MeasRRC_Inactive_r17_tags_5[] = {
	(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_dl_PRS_MeasRRC_Inactive_r17_5 = {
	"dl-PRS-MeasRRC-Inactive-r17",
	"dl-PRS-MeasRRC-Inactive-r17",
	&asn_OP_NativeEnumerated,
	asn_DEF_dl_PRS_MeasRRC_Inactive_r17_tags_5,
	sizeof(asn_DEF_dl_PRS_MeasRRC_Inactive_r17_tags_5)
		/sizeof(asn_DEF_dl_PRS_MeasRRC_Inactive_r17_tags_5[0]) - 1, /* 1 */
	asn_DEF_dl_PRS_MeasRRC_Inactive_r17_tags_5,	/* Same as above */
	sizeof(asn_DEF_dl_PRS_MeasRRC_Inactive_r17_tags_5)
		/sizeof(asn_DEF_dl_PRS_MeasRRC_Inactive_r17_tags_5[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_dl_PRS_MeasRRC_Inactive_r17_constr_5,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_dl_PRS_MeasRRC_Inactive_r17_specs_5	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_DL_TDOA_MeasCapabilityPerBand_r17_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DL_TDOA_MeasCapabilityPerBand_r17, freqBandIndicatorNR_r17),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_FreqBandIndicatorNR_r16,
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
		"freqBandIndicatorNR-r17"
		},
	{ ATF_POINTER, 2, offsetof(struct DL_TDOA_MeasCapabilityPerBand_r17, supportOfDL_PRS_FirstPathRSRP_r17),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_supportOfDL_PRS_FirstPathRSRP_r17_3,
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
		"supportOfDL-PRS-FirstPathRSRP-r17"
		},
	{ ATF_POINTER, 1, offsetof(struct DL_TDOA_MeasCapabilityPerBand_r17, dl_PRS_MeasRRC_Inactive_r17),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_dl_PRS_MeasRRC_Inactive_r17_5,
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
		"dl-PRS-MeasRRC-Inactive-r17"
		},
};
static const int asn_MAP_DL_TDOA_MeasCapabilityPerBand_r17_oms_1[] = { 1, 2 };
static const ber_tlv_tag_t asn_DEF_DL_TDOA_MeasCapabilityPerBand_r17_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_DL_TDOA_MeasCapabilityPerBand_r17_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* freqBandIndicatorNR-r17 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* supportOfDL-PRS-FirstPathRSRP-r17 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* dl-PRS-MeasRRC-Inactive-r17 */
};
asn_SEQUENCE_specifics_t asn_SPC_DL_TDOA_MeasCapabilityPerBand_r17_specs_1 = {
	sizeof(struct DL_TDOA_MeasCapabilityPerBand_r17),
	offsetof(struct DL_TDOA_MeasCapabilityPerBand_r17, _asn_ctx),
	asn_MAP_DL_TDOA_MeasCapabilityPerBand_r17_tag2el_1,
	3,	/* Count of tags in the map */
	asn_MAP_DL_TDOA_MeasCapabilityPerBand_r17_oms_1,	/* Optional members */
	2, 0,	/* Root/Additions */
	3,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_DL_TDOA_MeasCapabilityPerBand_r17 = {
	"DL-TDOA-MeasCapabilityPerBand-r17",
	"DL-TDOA-MeasCapabilityPerBand-r17",
	&asn_OP_SEQUENCE,
	asn_DEF_DL_TDOA_MeasCapabilityPerBand_r17_tags_1,
	sizeof(asn_DEF_DL_TDOA_MeasCapabilityPerBand_r17_tags_1)
		/sizeof(asn_DEF_DL_TDOA_MeasCapabilityPerBand_r17_tags_1[0]), /* 1 */
	asn_DEF_DL_TDOA_MeasCapabilityPerBand_r17_tags_1,	/* Same as above */
	sizeof(asn_DEF_DL_TDOA_MeasCapabilityPerBand_r17_tags_1)
		/sizeof(asn_DEF_DL_TDOA_MeasCapabilityPerBand_r17_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_DL_TDOA_MeasCapabilityPerBand_r17_1,
	3,	/* Elements count */
	&asn_SPC_DL_TDOA_MeasCapabilityPerBand_r17_specs_1	/* Additional specs */
};

