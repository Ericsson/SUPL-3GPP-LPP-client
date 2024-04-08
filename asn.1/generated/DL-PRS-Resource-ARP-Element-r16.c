/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "DL-PRS-Resource-ARP-Element-r16.h"

#include "RelativeLocation-r16.h"
asn_TYPE_member_t asn_MBR_DL_PRS_Resource_ARP_Element_r16_1[] = {
	{ ATF_POINTER, 1, offsetof(struct DL_PRS_Resource_ARP_Element_r16, dl_PRS_Resource_ARP_location_r16),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RelativeLocation_r16,
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
		"dl-PRS-Resource-ARP-location-r16"
		},
};
static const int asn_MAP_DL_PRS_Resource_ARP_Element_r16_oms_1[] = { 0 };
static const ber_tlv_tag_t asn_DEF_DL_PRS_Resource_ARP_Element_r16_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_DL_PRS_Resource_ARP_Element_r16_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* dl-PRS-Resource-ARP-location-r16 */
};
asn_SEQUENCE_specifics_t asn_SPC_DL_PRS_Resource_ARP_Element_r16_specs_1 = {
	sizeof(struct DL_PRS_Resource_ARP_Element_r16),
	offsetof(struct DL_PRS_Resource_ARP_Element_r16, _asn_ctx),
	asn_MAP_DL_PRS_Resource_ARP_Element_r16_tag2el_1,
	1,	/* Count of tags in the map */
	asn_MAP_DL_PRS_Resource_ARP_Element_r16_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_DL_PRS_Resource_ARP_Element_r16 = {
	"DL-PRS-Resource-ARP-Element-r16",
	"DL-PRS-Resource-ARP-Element-r16",
	&asn_OP_SEQUENCE,
	asn_DEF_DL_PRS_Resource_ARP_Element_r16_tags_1,
	sizeof(asn_DEF_DL_PRS_Resource_ARP_Element_r16_tags_1)
		/sizeof(asn_DEF_DL_PRS_Resource_ARP_Element_r16_tags_1[0]), /* 1 */
	asn_DEF_DL_PRS_Resource_ARP_Element_r16_tags_1,	/* Same as above */
	sizeof(asn_DEF_DL_PRS_Resource_ARP_Element_r16_tags_1)
		/sizeof(asn_DEF_DL_PRS_Resource_ARP_Element_r16_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_DL_PRS_Resource_ARP_Element_r16_1,
	1,	/* Elements count */
	&asn_SPC_DL_PRS_Resource_ARP_Element_r16_specs_1	/* Additional specs */
};

