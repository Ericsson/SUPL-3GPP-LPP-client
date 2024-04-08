/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Ver2-ULP-Components"
 * 	found in "src/ULP-Components.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "SupportedWCDMAInfo.h"

asn_TYPE_member_t asn_MBR_SupportedWCDMAInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SupportedWCDMAInfo, mRL),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
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
		"mRL"
		},
};
static const ber_tlv_tag_t asn_DEF_SupportedWCDMAInfo_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_SupportedWCDMAInfo_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* mRL */
};
asn_SEQUENCE_specifics_t asn_SPC_SupportedWCDMAInfo_specs_1 = {
	sizeof(struct SupportedWCDMAInfo),
	offsetof(struct SupportedWCDMAInfo, _asn_ctx),
	asn_MAP_SupportedWCDMAInfo_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_SupportedWCDMAInfo = {
	"SupportedWCDMAInfo",
	"SupportedWCDMAInfo",
	&asn_OP_SEQUENCE,
	asn_DEF_SupportedWCDMAInfo_tags_1,
	sizeof(asn_DEF_SupportedWCDMAInfo_tags_1)
		/sizeof(asn_DEF_SupportedWCDMAInfo_tags_1[0]), /* 1 */
	asn_DEF_SupportedWCDMAInfo_tags_1,	/* Same as above */
	sizeof(asn_DEF_SupportedWCDMAInfo_tags_1)
		/sizeof(asn_DEF_SupportedWCDMAInfo_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_SupportedWCDMAInfo_1,
	1,	/* Elements count */
	&asn_SPC_SupportedWCDMAInfo_specs_1	/* Additional specs */
};

