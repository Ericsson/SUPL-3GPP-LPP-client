/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "DL-PRS-BeamInfoResourceSet-r16.h"

#include "DL-PRS-BeamInfoElement-r16.h"
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
asn_per_constraints_t asn_PER_type_DL_PRS_BeamInfoResourceSet_r16_constr_1 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 6,  6,  1,  64 }	/* (SIZE(1..64)) */,
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
asn_TYPE_member_t asn_MBR_DL_PRS_BeamInfoResourceSet_r16_1[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_DL_PRS_BeamInfoElement_r16,
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
		""
		},
};
static const ber_tlv_tag_t asn_DEF_DL_PRS_BeamInfoResourceSet_r16_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
asn_SET_OF_specifics_t asn_SPC_DL_PRS_BeamInfoResourceSet_r16_specs_1 = {
	sizeof(struct DL_PRS_BeamInfoResourceSet_r16),
	offsetof(struct DL_PRS_BeamInfoResourceSet_r16, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
asn_TYPE_descriptor_t asn_DEF_DL_PRS_BeamInfoResourceSet_r16 = {
	"DL-PRS-BeamInfoResourceSet-r16",
	"DL-PRS-BeamInfoResourceSet-r16",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_DL_PRS_BeamInfoResourceSet_r16_tags_1,
	sizeof(asn_DEF_DL_PRS_BeamInfoResourceSet_r16_tags_1)
		/sizeof(asn_DEF_DL_PRS_BeamInfoResourceSet_r16_tags_1[0]), /* 1 */
	asn_DEF_DL_PRS_BeamInfoResourceSet_r16_tags_1,	/* Same as above */
	sizeof(asn_DEF_DL_PRS_BeamInfoResourceSet_r16_tags_1)
		/sizeof(asn_DEF_DL_PRS_BeamInfoResourceSet_r16_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_DL_PRS_BeamInfoResourceSet_r16_constr_1,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_OF_constraint
	},
	asn_MBR_DL_PRS_BeamInfoResourceSet_r16_1,
	1,	/* Single element */
	&asn_SPC_DL_PRS_BeamInfoResourceSet_r16_specs_1	/* Additional specs */
};

