/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "WLAN-AP-Location-r14.h"

asn_TYPE_member_t asn_MBR_WLAN_AP_Location_r14_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct WLAN_AP_Location_r14, locationDataLCI_r14),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LocationDataLCI_r14,
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
		"locationDataLCI-r14"
		},
};
static const ber_tlv_tag_t asn_DEF_WLAN_AP_Location_r14_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_WLAN_AP_Location_r14_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* locationDataLCI-r14 */
};
asn_SEQUENCE_specifics_t asn_SPC_WLAN_AP_Location_r14_specs_1 = {
	sizeof(struct WLAN_AP_Location_r14),
	offsetof(struct WLAN_AP_Location_r14, _asn_ctx),
	asn_MAP_WLAN_AP_Location_r14_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_WLAN_AP_Location_r14 = {
	"WLAN-AP-Location-r14",
	"WLAN-AP-Location-r14",
	&asn_OP_SEQUENCE,
	asn_DEF_WLAN_AP_Location_r14_tags_1,
	sizeof(asn_DEF_WLAN_AP_Location_r14_tags_1)
		/sizeof(asn_DEF_WLAN_AP_Location_r14_tags_1[0]), /* 1 */
	asn_DEF_WLAN_AP_Location_r14_tags_1,	/* Same as above */
	sizeof(asn_DEF_WLAN_AP_Location_r14_tags_1)
		/sizeof(asn_DEF_WLAN_AP_Location_r14_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_WLAN_AP_Location_r14_1,
	1,	/* Elements count */
	&asn_SPC_WLAN_AP_Location_r14_specs_1	/* Additional specs */
};

