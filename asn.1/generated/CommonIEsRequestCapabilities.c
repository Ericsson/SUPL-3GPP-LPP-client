/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "CommonIEsRequestCapabilities.h"

static asn_TYPE_member_t asn_MBR_ext1_3[] = {
	{ ATF_POINTER, 1, offsetof(struct CommonIEsRequestCapabilities__ext1, lpp_message_segmentation_req_r14),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BIT_STRING,
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
		"lpp-message-segmentation-req-r14"
		},
};
static const int asn_MAP_ext1_oms_3[] = { 0 };
static const ber_tlv_tag_t asn_DEF_ext1_tags_3[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext1_tag2el_3[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* lpp-message-segmentation-req-r14 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext1_specs_3 = {
	sizeof(struct CommonIEsRequestCapabilities__ext1),
	offsetof(struct CommonIEsRequestCapabilities__ext1, _asn_ctx),
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

asn_TYPE_member_t asn_MBR_CommonIEsRequestCapabilities_1[] = {
	{ ATF_POINTER, 1, offsetof(struct CommonIEsRequestCapabilities, ext1),
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
static const int asn_MAP_CommonIEsRequestCapabilities_oms_1[] = { 0 };
static const ber_tlv_tag_t asn_DEF_CommonIEsRequestCapabilities_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_CommonIEsRequestCapabilities_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* ext1 */
};
asn_SEQUENCE_specifics_t asn_SPC_CommonIEsRequestCapabilities_specs_1 = {
	sizeof(struct CommonIEsRequestCapabilities),
	offsetof(struct CommonIEsRequestCapabilities, _asn_ctx),
	asn_MAP_CommonIEsRequestCapabilities_tag2el_1,
	1,	/* Count of tags in the map */
	asn_MAP_CommonIEsRequestCapabilities_oms_1,	/* Optional members */
	0, 1,	/* Root/Additions */
	0,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_CommonIEsRequestCapabilities = {
	"CommonIEsRequestCapabilities",
	"CommonIEsRequestCapabilities",
	&asn_OP_SEQUENCE,
	asn_DEF_CommonIEsRequestCapabilities_tags_1,
	sizeof(asn_DEF_CommonIEsRequestCapabilities_tags_1)
		/sizeof(asn_DEF_CommonIEsRequestCapabilities_tags_1[0]), /* 1 */
	asn_DEF_CommonIEsRequestCapabilities_tags_1,	/* Same as above */
	sizeof(asn_DEF_CommonIEsRequestCapabilities_tags_1)
		/sizeof(asn_DEF_CommonIEsRequestCapabilities_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_CommonIEsRequestCapabilities_1,
	1,	/* Elements count */
	&asn_SPC_CommonIEsRequestCapabilities_specs_1	/* Additional specs */
};

