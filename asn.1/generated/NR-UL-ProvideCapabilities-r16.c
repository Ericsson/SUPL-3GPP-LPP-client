/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "NR-UL-ProvideCapabilities-r16.h"

#include "NR-UE-TEG-Capability-r17.h"
static asn_TYPE_member_t asn_MBR_ext1_4[] = {
	{ ATF_POINTER, 1, offsetof(struct NR_UL_ProvideCapabilities_r16__ext1, nr_UE_TEG_Capability_r17),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NR_UE_TEG_Capability_r17,
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
		"nr-UE-TEG-Capability-r17"
		},
};
static const int asn_MAP_ext1_oms_4[] = { 0 };
static const ber_tlv_tag_t asn_DEF_ext1_tags_4[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext1_tag2el_4[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* nr-UE-TEG-Capability-r17 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext1_specs_4 = {
	sizeof(struct NR_UL_ProvideCapabilities_r16__ext1),
	offsetof(struct NR_UL_ProvideCapabilities_r16__ext1, _asn_ctx),
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

asn_TYPE_member_t asn_MBR_NR_UL_ProvideCapabilities_r16_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct NR_UL_ProvideCapabilities_r16, nr_UL_SRS_Capability_r16),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NR_UL_SRS_Capability_r16,
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
		"nr-UL-SRS-Capability-r16"
		},
	{ ATF_POINTER, 1, offsetof(struct NR_UL_ProvideCapabilities_r16, ext1),
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
static const int asn_MAP_NR_UL_ProvideCapabilities_r16_oms_1[] = { 1 };
static const ber_tlv_tag_t asn_DEF_NR_UL_ProvideCapabilities_r16_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_NR_UL_ProvideCapabilities_r16_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* nr-UL-SRS-Capability-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* ext1 */
};
asn_SEQUENCE_specifics_t asn_SPC_NR_UL_ProvideCapabilities_r16_specs_1 = {
	sizeof(struct NR_UL_ProvideCapabilities_r16),
	offsetof(struct NR_UL_ProvideCapabilities_r16, _asn_ctx),
	asn_MAP_NR_UL_ProvideCapabilities_r16_tag2el_1,
	2,	/* Count of tags in the map */
	asn_MAP_NR_UL_ProvideCapabilities_r16_oms_1,	/* Optional members */
	0, 1,	/* Root/Additions */
	1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_NR_UL_ProvideCapabilities_r16 = {
	"NR-UL-ProvideCapabilities-r16",
	"NR-UL-ProvideCapabilities-r16",
	&asn_OP_SEQUENCE,
	asn_DEF_NR_UL_ProvideCapabilities_r16_tags_1,
	sizeof(asn_DEF_NR_UL_ProvideCapabilities_r16_tags_1)
		/sizeof(asn_DEF_NR_UL_ProvideCapabilities_r16_tags_1[0]), /* 1 */
	asn_DEF_NR_UL_ProvideCapabilities_r16_tags_1,	/* Same as above */
	sizeof(asn_DEF_NR_UL_ProvideCapabilities_r16_tags_1)
		/sizeof(asn_DEF_NR_UL_ProvideCapabilities_r16_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_NR_UL_ProvideCapabilities_r16_1,
	2,	/* Elements count */
	&asn_SPC_NR_UL_ProvideCapabilities_r16_specs_1	/* Additional specs */
};

