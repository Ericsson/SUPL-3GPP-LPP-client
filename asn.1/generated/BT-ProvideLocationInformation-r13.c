/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "BT-ProvideLocationInformation-r13.h"

#include "BT-MeasurementInformation-r13.h"
#include "BT-Error-r13.h"
asn_TYPE_member_t asn_MBR_BT_ProvideLocationInformation_r13_1[] = {
	{ ATF_POINTER, 2, offsetof(struct BT_ProvideLocationInformation_r13, bt_MeasurementInformation_r13),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BT_MeasurementInformation_r13,
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
		"bt-MeasurementInformation-r13"
		},
	{ ATF_POINTER, 1, offsetof(struct BT_ProvideLocationInformation_r13, bt_Error_r13),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_BT_Error_r13,
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
		"bt-Error-r13"
		},
};
static const int asn_MAP_BT_ProvideLocationInformation_r13_oms_1[] = { 0, 1 };
static const ber_tlv_tag_t asn_DEF_BT_ProvideLocationInformation_r13_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_BT_ProvideLocationInformation_r13_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* bt-MeasurementInformation-r13 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* bt-Error-r13 */
};
asn_SEQUENCE_specifics_t asn_SPC_BT_ProvideLocationInformation_r13_specs_1 = {
	sizeof(struct BT_ProvideLocationInformation_r13),
	offsetof(struct BT_ProvideLocationInformation_r13, _asn_ctx),
	asn_MAP_BT_ProvideLocationInformation_r13_tag2el_1,
	2,	/* Count of tags in the map */
	asn_MAP_BT_ProvideLocationInformation_r13_oms_1,	/* Optional members */
	2, 0,	/* Root/Additions */
	2,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_BT_ProvideLocationInformation_r13 = {
	"BT-ProvideLocationInformation-r13",
	"BT-ProvideLocationInformation-r13",
	&asn_OP_SEQUENCE,
	asn_DEF_BT_ProvideLocationInformation_r13_tags_1,
	sizeof(asn_DEF_BT_ProvideLocationInformation_r13_tags_1)
		/sizeof(asn_DEF_BT_ProvideLocationInformation_r13_tags_1[0]), /* 1 */
	asn_DEF_BT_ProvideLocationInformation_r13_tags_1,	/* Same as above */
	sizeof(asn_DEF_BT_ProvideLocationInformation_r13_tags_1)
		/sizeof(asn_DEF_BT_ProvideLocationInformation_r13_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_BT_ProvideLocationInformation_r13_1,
	2,	/* Elements count */
	&asn_SPC_BT_ProvideLocationInformation_r13_specs_1	/* Additional specs */
};

