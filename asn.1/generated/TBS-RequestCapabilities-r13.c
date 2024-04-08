/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "TBS-RequestCapabilities-r13.h"

static const ber_tlv_tag_t asn_DEF_TBS_RequestCapabilities_r13_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
asn_SEQUENCE_specifics_t asn_SPC_TBS_RequestCapabilities_r13_specs_1 = {
	sizeof(struct TBS_RequestCapabilities_r13),
	offsetof(struct TBS_RequestCapabilities_r13, _asn_ctx),
	0,	/* No top level tags */
	0,	/* No tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	0,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_TBS_RequestCapabilities_r13 = {
	"TBS-RequestCapabilities-r13",
	"TBS-RequestCapabilities-r13",
	&asn_OP_SEQUENCE,
	asn_DEF_TBS_RequestCapabilities_r13_tags_1,
	sizeof(asn_DEF_TBS_RequestCapabilities_r13_tags_1)
		/sizeof(asn_DEF_TBS_RequestCapabilities_r13_tags_1[0]), /* 1 */
	asn_DEF_TBS_RequestCapabilities_r13_tags_1,	/* Same as above */
	sizeof(asn_DEF_TBS_RequestCapabilities_r13_tags_1)
		/sizeof(asn_DEF_TBS_RequestCapabilities_r13_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	0, 0,	/* No members */
	&asn_SPC_TBS_RequestCapabilities_r13_specs_1	/* Additional specs */
};

