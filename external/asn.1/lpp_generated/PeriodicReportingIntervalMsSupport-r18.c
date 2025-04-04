/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#include "PeriodicReportingIntervalMsSupport-r18.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_minPeriodicReportingIntervalMs_r18_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  2,  2,  0,  2 }	/* (0..2,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static const asn_INTEGER_enum_map_t asn_MAP_minPeriodicReportingIntervalMs_r18_value2enum_2[] = {
	{ 0,	3,	"ms1" },
	{ 1,	4,	"ms10" },
	{ 2,	5,	"ms100" }
	/* This list is extensible */
};
static const unsigned int asn_MAP_minPeriodicReportingIntervalMs_r18_enum2value_2[] = {
	0,	/* ms1(0) */
	1,	/* ms10(1) */
	2	/* ms100(2) */
	/* This list is extensible */
};
static const asn_INTEGER_specifics_t asn_SPC_minPeriodicReportingIntervalMs_r18_specs_2 = {
	asn_MAP_minPeriodicReportingIntervalMs_r18_value2enum_2,	/* "tag" => N; sorted by tag */
	asn_MAP_minPeriodicReportingIntervalMs_r18_enum2value_2,	/* N => "tag"; sorted by N */
	3,	/* Number of elements in the maps */
	4,	/* Extensions before this member */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_minPeriodicReportingIntervalMs_r18_tags_2[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_minPeriodicReportingIntervalMs_r18_2 = {
	"minPeriodicReportingIntervalMs-r18",
	"minPeriodicReportingIntervalMs-r18",
	&asn_OP_NativeEnumerated,
	asn_DEF_minPeriodicReportingIntervalMs_r18_tags_2,
	sizeof(asn_DEF_minPeriodicReportingIntervalMs_r18_tags_2)
		/sizeof(asn_DEF_minPeriodicReportingIntervalMs_r18_tags_2[0]) - 1, /* 1 */
	asn_DEF_minPeriodicReportingIntervalMs_r18_tags_2,	/* Same as above */
	sizeof(asn_DEF_minPeriodicReportingIntervalMs_r18_tags_2)
		/sizeof(asn_DEF_minPeriodicReportingIntervalMs_r18_tags_2[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_minPeriodicReportingIntervalMs_r18_constr_2,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_minPeriodicReportingIntervalMs_r18_specs_2	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_PeriodicReportingIntervalMsSupport_r18_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PeriodicReportingIntervalMsSupport_r18, minPeriodicReportingIntervalMs_r18),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_minPeriodicReportingIntervalMs_r18_2,
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
		"minPeriodicReportingIntervalMs-r18"
		},
};
static const ber_tlv_tag_t asn_DEF_PeriodicReportingIntervalMsSupport_r18_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_PeriodicReportingIntervalMsSupport_r18_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* minPeriodicReportingIntervalMs-r18 */
};
asn_SEQUENCE_specifics_t asn_SPC_PeriodicReportingIntervalMsSupport_r18_specs_1 = {
	sizeof(struct PeriodicReportingIntervalMsSupport_r18),
	offsetof(struct PeriodicReportingIntervalMsSupport_r18, _asn_ctx),
	asn_MAP_PeriodicReportingIntervalMsSupport_r18_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_PeriodicReportingIntervalMsSupport_r18 = {
	"PeriodicReportingIntervalMsSupport-r18",
	"PeriodicReportingIntervalMsSupport-r18",
	&asn_OP_SEQUENCE,
	asn_DEF_PeriodicReportingIntervalMsSupport_r18_tags_1,
	sizeof(asn_DEF_PeriodicReportingIntervalMsSupport_r18_tags_1)
		/sizeof(asn_DEF_PeriodicReportingIntervalMsSupport_r18_tags_1[0]), /* 1 */
	asn_DEF_PeriodicReportingIntervalMsSupport_r18_tags_1,	/* Same as above */
	sizeof(asn_DEF_PeriodicReportingIntervalMsSupport_r18_tags_1)
		/sizeof(asn_DEF_PeriodicReportingIntervalMsSupport_r18_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_PeriodicReportingIntervalMsSupport_r18_1,
	1,	/* Elements count */
	&asn_SPC_PeriodicReportingIntervalMsSupport_r18_specs_1	/* Additional specs */
};

