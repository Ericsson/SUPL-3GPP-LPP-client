/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "GNSS-SSR-GriddedCorrectionSupport-r16.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_griddedCorrectionIntegritySup_r17_constr_4 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 0,  0,  0,  0 }	/* (0..0) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static const asn_INTEGER_enum_map_t asn_MAP_griddedCorrectionIntegritySup_r17_value2enum_4[] = {
	{ 0,	9,	"supported" }
};
static const unsigned int asn_MAP_griddedCorrectionIntegritySup_r17_enum2value_4[] = {
	0	/* supported(0) */
};
static const asn_INTEGER_specifics_t asn_SPC_griddedCorrectionIntegritySup_r17_specs_4 = {
	asn_MAP_griddedCorrectionIntegritySup_r17_value2enum_4,	/* "tag" => N; sorted by tag */
	asn_MAP_griddedCorrectionIntegritySup_r17_enum2value_4,	/* N => "tag"; sorted by N */
	1,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_griddedCorrectionIntegritySup_r17_tags_4[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_griddedCorrectionIntegritySup_r17_4 = {
	"griddedCorrectionIntegritySup-r17",
	"griddedCorrectionIntegritySup-r17",
	&asn_OP_NativeEnumerated,
	asn_DEF_griddedCorrectionIntegritySup_r17_tags_4,
	sizeof(asn_DEF_griddedCorrectionIntegritySup_r17_tags_4)
		/sizeof(asn_DEF_griddedCorrectionIntegritySup_r17_tags_4[0]) - 1, /* 1 */
	asn_DEF_griddedCorrectionIntegritySup_r17_tags_4,	/* Same as above */
	sizeof(asn_DEF_griddedCorrectionIntegritySup_r17_tags_4)
		/sizeof(asn_DEF_griddedCorrectionIntegritySup_r17_tags_4[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_griddedCorrectionIntegritySup_r17_constr_4,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_griddedCorrectionIntegritySup_r17_specs_4	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_ext1_3[] = {
	{ ATF_POINTER, 1, offsetof(struct GNSS_SSR_GriddedCorrectionSupport_r16__ext1, griddedCorrectionIntegritySup_r17),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_griddedCorrectionIntegritySup_r17_4,
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
		"griddedCorrectionIntegritySup-r17"
		},
};
static const int asn_MAP_ext1_oms_3[] = { 0 };
static const ber_tlv_tag_t asn_DEF_ext1_tags_3[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext1_tag2el_3[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* griddedCorrectionIntegritySup-r17 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext1_specs_3 = {
	sizeof(struct GNSS_SSR_GriddedCorrectionSupport_r16__ext1),
	offsetof(struct GNSS_SSR_GriddedCorrectionSupport_r16__ext1, _asn_ctx),
	asn_MAP_ext1_tag2el_3,
	1,	/* Count of tags in the map */
	asn_MAP_ext1_oms_3,	/* Optional members */
	1, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_ext1_3 = {
	"ext1",
	"ext1",
	&asn_OP_SEQUENCE,
	asn_DEF_ext1_tags_3,
	sizeof(asn_DEF_ext1_tags_3)
		/sizeof(asn_DEF_ext1_tags_3[0]) - 1, /* 1 */
	asn_DEF_ext1_tags_3,	/* Same as above */
	sizeof(asn_DEF_ext1_tags_3)
		/sizeof(asn_DEF_ext1_tags_3[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_ext1_3,
	1,	/* Elements count */
	&asn_SPC_ext1_specs_3	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_GNSS_SSR_GriddedCorrectionSupport_r16_1[] = {
	{ ATF_POINTER, 1, offsetof(struct GNSS_SSR_GriddedCorrectionSupport_r16, ext1),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		0,
		&asn_DEF_ext1_3,
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
static const int asn_MAP_GNSS_SSR_GriddedCorrectionSupport_r16_oms_1[] = { 0 };
static const ber_tlv_tag_t asn_DEF_GNSS_SSR_GriddedCorrectionSupport_r16_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_GNSS_SSR_GriddedCorrectionSupport_r16_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* ext1 */
};
asn_SEQUENCE_specifics_t asn_SPC_GNSS_SSR_GriddedCorrectionSupport_r16_specs_1 = {
	sizeof(struct GNSS_SSR_GriddedCorrectionSupport_r16),
	offsetof(struct GNSS_SSR_GriddedCorrectionSupport_r16, _asn_ctx),
	asn_MAP_GNSS_SSR_GriddedCorrectionSupport_r16_tag2el_1,
	1,	/* Count of tags in the map */
	asn_MAP_GNSS_SSR_GriddedCorrectionSupport_r16_oms_1,	/* Optional members */
	0, 1,	/* Root/Additions */
	0,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_GNSS_SSR_GriddedCorrectionSupport_r16 = {
	"GNSS-SSR-GriddedCorrectionSupport-r16",
	"GNSS-SSR-GriddedCorrectionSupport-r16",
	&asn_OP_SEQUENCE,
	asn_DEF_GNSS_SSR_GriddedCorrectionSupport_r16_tags_1,
	sizeof(asn_DEF_GNSS_SSR_GriddedCorrectionSupport_r16_tags_1)
		/sizeof(asn_DEF_GNSS_SSR_GriddedCorrectionSupport_r16_tags_1[0]), /* 1 */
	asn_DEF_GNSS_SSR_GriddedCorrectionSupport_r16_tags_1,	/* Same as above */
	sizeof(asn_DEF_GNSS_SSR_GriddedCorrectionSupport_r16_tags_1)
		/sizeof(asn_DEF_GNSS_SSR_GriddedCorrectionSupport_r16_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_GNSS_SSR_GriddedCorrectionSupport_r16_1,
	1,	/* Elements count */
	&asn_SPC_GNSS_SSR_GriddedCorrectionSupport_r16_specs_1	/* Additional specs */
};

