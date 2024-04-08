/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ULP-Version-2-parameter-extensions"
 * 	found in "src/SUPL.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "Ver2-PosProtocol-extension.h"

#include "PosProtocolVersion3GPP.h"
#include "PosProtocolVersion3GPP2.h"
asn_TYPE_member_t asn_MBR_Ver2_PosProtocol_extension_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Ver2_PosProtocol_extension, lpp),
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
		"lpp"
		},
	{ ATF_POINTER, 4, offsetof(struct Ver2_PosProtocol_extension, posProtocolVersionRRLP),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PosProtocolVersion3GPP,
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
		"posProtocolVersionRRLP"
		},
	{ ATF_POINTER, 3, offsetof(struct Ver2_PosProtocol_extension, posProtocolVersionRRC),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PosProtocolVersion3GPP,
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
		"posProtocolVersionRRC"
		},
	{ ATF_POINTER, 2, offsetof(struct Ver2_PosProtocol_extension, posProtocolVersionTIA801),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PosProtocolVersion3GPP2,
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
		"posProtocolVersionTIA801"
		},
	{ ATF_POINTER, 1, offsetof(struct Ver2_PosProtocol_extension, posProtocolVersionLPP),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PosProtocolVersion3GPP,
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
		"posProtocolVersionLPP"
		},
};
static const int asn_MAP_Ver2_PosProtocol_extension_oms_1[] = { 1, 2, 3, 4 };
static const ber_tlv_tag_t asn_DEF_Ver2_PosProtocol_extension_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_Ver2_PosProtocol_extension_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* lpp */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* posProtocolVersionRRLP */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* posProtocolVersionRRC */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* posProtocolVersionTIA801 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* posProtocolVersionLPP */
};
asn_SEQUENCE_specifics_t asn_SPC_Ver2_PosProtocol_extension_specs_1 = {
	sizeof(struct Ver2_PosProtocol_extension),
	offsetof(struct Ver2_PosProtocol_extension, _asn_ctx),
	asn_MAP_Ver2_PosProtocol_extension_tag2el_1,
	5,	/* Count of tags in the map */
	asn_MAP_Ver2_PosProtocol_extension_oms_1,	/* Optional members */
	4, 0,	/* Root/Additions */
	5,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_Ver2_PosProtocol_extension = {
	"Ver2-PosProtocol-extension",
	"Ver2-PosProtocol-extension",
	&asn_OP_SEQUENCE,
	asn_DEF_Ver2_PosProtocol_extension_tags_1,
	sizeof(asn_DEF_Ver2_PosProtocol_extension_tags_1)
		/sizeof(asn_DEF_Ver2_PosProtocol_extension_tags_1[0]), /* 1 */
	asn_DEF_Ver2_PosProtocol_extension_tags_1,	/* Same as above */
	sizeof(asn_DEF_Ver2_PosProtocol_extension_tags_1)
		/sizeof(asn_DEF_Ver2_PosProtocol_extension_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_Ver2_PosProtocol_extension_1,
	5,	/* Elements count */
	&asn_SPC_Ver2_PosProtocol_extension_specs_1	/* Additional specs */
};

