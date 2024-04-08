/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "NR-DL-TDOA-TargetDeviceErrorCauses-r16.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_cause_r16_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  3,  3,  0,  5 }	/* (0..5,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static const asn_INTEGER_enum_map_t asn_MAP_cause_r16_value2enum_2[] = {
	{ 0,	9,	"undefined" },
	{ 1,	23,	"assistance-data-missing" },
	{ 2,	21,	"unableToMeasureAnyTRP" },
	{ 3,	44,	"attemptedButUnableToMeasureSomeNeighbourTRPs" },
	{ 4,	50,	"thereWereNotEnoughSignalsReceivedForUeBasedDL-TDOA" },
	{ 5,	40,	"locationCalculationAssistanceDataMissing" }
	/* This list is extensible */
};
static const unsigned int asn_MAP_cause_r16_enum2value_2[] = {
	1,	/* assistance-data-missing(1) */
	3,	/* attemptedButUnableToMeasureSomeNeighbourTRPs(3) */
	5,	/* locationCalculationAssistanceDataMissing(5) */
	4,	/* thereWereNotEnoughSignalsReceivedForUeBasedDL-TDOA(4) */
	2,	/* unableToMeasureAnyTRP(2) */
	0	/* undefined(0) */
	/* This list is extensible */
};
static const asn_INTEGER_specifics_t asn_SPC_cause_r16_specs_2 = {
	asn_MAP_cause_r16_value2enum_2,	/* "tag" => N; sorted by tag */
	asn_MAP_cause_r16_enum2value_2,	/* N => "tag"; sorted by N */
	6,	/* Number of elements in the maps */
	7,	/* Extensions before this member */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_cause_r16_tags_2[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_cause_r16_2 = {
	"cause-r16",
	"cause-r16",
	&asn_OP_NativeEnumerated,
	asn_DEF_cause_r16_tags_2,
	sizeof(asn_DEF_cause_r16_tags_2)
		/sizeof(asn_DEF_cause_r16_tags_2[0]) - 1, /* 1 */
	asn_DEF_cause_r16_tags_2,	/* Same as above */
	sizeof(asn_DEF_cause_r16_tags_2)
		/sizeof(asn_DEF_cause_r16_tags_2[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_cause_r16_constr_2,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_cause_r16_specs_2	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_NR_DL_TDOA_TargetDeviceErrorCauses_r16_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct NR_DL_TDOA_TargetDeviceErrorCauses_r16, cause_r16),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_cause_r16_2,
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
		"cause-r16"
		},
};
static const ber_tlv_tag_t asn_DEF_NR_DL_TDOA_TargetDeviceErrorCauses_r16_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_NR_DL_TDOA_TargetDeviceErrorCauses_r16_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* cause-r16 */
};
asn_SEQUENCE_specifics_t asn_SPC_NR_DL_TDOA_TargetDeviceErrorCauses_r16_specs_1 = {
	sizeof(struct NR_DL_TDOA_TargetDeviceErrorCauses_r16),
	offsetof(struct NR_DL_TDOA_TargetDeviceErrorCauses_r16, _asn_ctx),
	asn_MAP_NR_DL_TDOA_TargetDeviceErrorCauses_r16_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_NR_DL_TDOA_TargetDeviceErrorCauses_r16 = {
	"NR-DL-TDOA-TargetDeviceErrorCauses-r16",
	"NR-DL-TDOA-TargetDeviceErrorCauses-r16",
	&asn_OP_SEQUENCE,
	asn_DEF_NR_DL_TDOA_TargetDeviceErrorCauses_r16_tags_1,
	sizeof(asn_DEF_NR_DL_TDOA_TargetDeviceErrorCauses_r16_tags_1)
		/sizeof(asn_DEF_NR_DL_TDOA_TargetDeviceErrorCauses_r16_tags_1[0]), /* 1 */
	asn_DEF_NR_DL_TDOA_TargetDeviceErrorCauses_r16_tags_1,	/* Same as above */
	sizeof(asn_DEF_NR_DL_TDOA_TargetDeviceErrorCauses_r16_tags_1)
		/sizeof(asn_DEF_NR_DL_TDOA_TargetDeviceErrorCauses_r16_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_NR_DL_TDOA_TargetDeviceErrorCauses_r16_1,
	1,	/* Elements count */
	&asn_SPC_NR_DL_TDOA_TargetDeviceErrorCauses_r16_specs_1	/* Additional specs */
};

