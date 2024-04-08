/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "GNSS-SSR-PhaseBiasSupport-r16.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_ssr_IntegrityPhaseBiasBoundsSup_r17_constr_5 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 0,  0,  0,  0 }	/* (0..0) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static const asn_INTEGER_enum_map_t asn_MAP_ssr_IntegrityPhaseBiasBoundsSup_r17_value2enum_5[] = {
	{ 0,	9,	"supported" }
};
static const unsigned int asn_MAP_ssr_IntegrityPhaseBiasBoundsSup_r17_enum2value_5[] = {
	0	/* supported(0) */
};
static const asn_INTEGER_specifics_t asn_SPC_ssr_IntegrityPhaseBiasBoundsSup_r17_specs_5 = {
	asn_MAP_ssr_IntegrityPhaseBiasBoundsSup_r17_value2enum_5,	/* "tag" => N; sorted by tag */
	asn_MAP_ssr_IntegrityPhaseBiasBoundsSup_r17_enum2value_5,	/* N => "tag"; sorted by N */
	1,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_ssr_IntegrityPhaseBiasBoundsSup_r17_tags_5[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_ssr_IntegrityPhaseBiasBoundsSup_r17_5 = {
	"ssr-IntegrityPhaseBiasBoundsSup-r17",
	"ssr-IntegrityPhaseBiasBoundsSup-r17",
	&asn_OP_NativeEnumerated,
	asn_DEF_ssr_IntegrityPhaseBiasBoundsSup_r17_tags_5,
	sizeof(asn_DEF_ssr_IntegrityPhaseBiasBoundsSup_r17_tags_5)
		/sizeof(asn_DEF_ssr_IntegrityPhaseBiasBoundsSup_r17_tags_5[0]) - 1, /* 1 */
	asn_DEF_ssr_IntegrityPhaseBiasBoundsSup_r17_tags_5,	/* Same as above */
	sizeof(asn_DEF_ssr_IntegrityPhaseBiasBoundsSup_r17_tags_5)
		/sizeof(asn_DEF_ssr_IntegrityPhaseBiasBoundsSup_r17_tags_5[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_ssr_IntegrityPhaseBiasBoundsSup_r17_constr_5,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_ssr_IntegrityPhaseBiasBoundsSup_r17_specs_5	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_ext1_4[] = {
	{ ATF_POINTER, 1, offsetof(struct GNSS_SSR_PhaseBiasSupport_r16__ext1, ssr_IntegrityPhaseBiasBoundsSup_r17),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ssr_IntegrityPhaseBiasBoundsSup_r17_5,
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
		"ssr-IntegrityPhaseBiasBoundsSup-r17"
		},
};
static const int asn_MAP_ext1_oms_4[] = { 0 };
static const ber_tlv_tag_t asn_DEF_ext1_tags_4[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext1_tag2el_4[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* ssr-IntegrityPhaseBiasBoundsSup-r17 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext1_specs_4 = {
	sizeof(struct GNSS_SSR_PhaseBiasSupport_r16__ext1),
	offsetof(struct GNSS_SSR_PhaseBiasSupport_r16__ext1, _asn_ctx),
	asn_MAP_ext1_tag2el_4,
	1,	/* Count of tags in the map */
	asn_MAP_ext1_oms_4,	/* Optional members */
	1, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_ext1_4 = {
	"ext1",
	"ext1",
	&asn_OP_SEQUENCE,
	asn_DEF_ext1_tags_4,
	sizeof(asn_DEF_ext1_tags_4)
		/sizeof(asn_DEF_ext1_tags_4[0]) - 1, /* 1 */
	asn_DEF_ext1_tags_4,	/* Same as above */
	sizeof(asn_DEF_ext1_tags_4)
		/sizeof(asn_DEF_ext1_tags_4[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_ext1_4,
	1,	/* Elements count */
	&asn_SPC_ext1_specs_4	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_GNSS_SSR_PhaseBiasSupport_r16_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct GNSS_SSR_PhaseBiasSupport_r16, signal_and_tracking_mode_ID_Sup_r16),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GNSS_SignalIDs,
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
		"signal-and-tracking-mode-ID-Sup-r16"
		},
	{ ATF_POINTER, 1, offsetof(struct GNSS_SSR_PhaseBiasSupport_r16, ext1),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		0,
		&asn_DEF_ext1_4,
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
static const int asn_MAP_GNSS_SSR_PhaseBiasSupport_r16_oms_1[] = { 1 };
static const ber_tlv_tag_t asn_DEF_GNSS_SSR_PhaseBiasSupport_r16_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_GNSS_SSR_PhaseBiasSupport_r16_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* signal-and-tracking-mode-ID-Sup-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* ext1 */
};
asn_SEQUENCE_specifics_t asn_SPC_GNSS_SSR_PhaseBiasSupport_r16_specs_1 = {
	sizeof(struct GNSS_SSR_PhaseBiasSupport_r16),
	offsetof(struct GNSS_SSR_PhaseBiasSupport_r16, _asn_ctx),
	asn_MAP_GNSS_SSR_PhaseBiasSupport_r16_tag2el_1,
	2,	/* Count of tags in the map */
	asn_MAP_GNSS_SSR_PhaseBiasSupport_r16_oms_1,	/* Optional members */
	0, 1,	/* Root/Additions */
	1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_GNSS_SSR_PhaseBiasSupport_r16 = {
	"GNSS-SSR-PhaseBiasSupport-r16",
	"GNSS-SSR-PhaseBiasSupport-r16",
	&asn_OP_SEQUENCE,
	asn_DEF_GNSS_SSR_PhaseBiasSupport_r16_tags_1,
	sizeof(asn_DEF_GNSS_SSR_PhaseBiasSupport_r16_tags_1)
		/sizeof(asn_DEF_GNSS_SSR_PhaseBiasSupport_r16_tags_1[0]), /* 1 */
	asn_DEF_GNSS_SSR_PhaseBiasSupport_r16_tags_1,	/* Same as above */
	sizeof(asn_DEF_GNSS_SSR_PhaseBiasSupport_r16_tags_1)
		/sizeof(asn_DEF_GNSS_SSR_PhaseBiasSupport_r16_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_GNSS_SSR_PhaseBiasSupport_r16_1,
	2,	/* Elements count */
	&asn_SPC_GNSS_SSR_PhaseBiasSupport_r16_specs_1	/* Additional specs */
};

