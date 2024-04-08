/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "WLAN-TargetDeviceErrorCauses-r13.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_cause_r13_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  2,  2,  0,  2 }	/* (0..2,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static const asn_INTEGER_enum_map_t asn_MAP_cause_r13_value2enum_2[] = {
	{ 0,	9,	"undefined" },
	{ 1,	33,	"requestedMeasurementsNotAvailable" },
	{ 2,	35,	"notAllrequestedMeasurementsPossible" }
	/* This list is extensible */
};
static const unsigned int asn_MAP_cause_r13_enum2value_2[] = {
	2,	/* notAllrequestedMeasurementsPossible(2) */
	1,	/* requestedMeasurementsNotAvailable(1) */
	0	/* undefined(0) */
	/* This list is extensible */
};
static const asn_INTEGER_specifics_t asn_SPC_cause_r13_specs_2 = {
	asn_MAP_cause_r13_value2enum_2,	/* "tag" => N; sorted by tag */
	asn_MAP_cause_r13_enum2value_2,	/* N => "tag"; sorted by N */
	3,	/* Number of elements in the maps */
	4,	/* Extensions before this member */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_cause_r13_tags_2[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_cause_r13_2 = {
	"cause-r13",
	"cause-r13",
	&asn_OP_NativeEnumerated,
	asn_DEF_cause_r13_tags_2,
	sizeof(asn_DEF_cause_r13_tags_2)
		/sizeof(asn_DEF_cause_r13_tags_2[0]) - 1, /* 1 */
	asn_DEF_cause_r13_tags_2,	/* Same as above */
	sizeof(asn_DEF_cause_r13_tags_2)
		/sizeof(asn_DEF_cause_r13_tags_2[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_cause_r13_constr_2,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_cause_r13_specs_2	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_WLAN_TargetDeviceErrorCauses_r13_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct WLAN_TargetDeviceErrorCauses_r13, cause_r13),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_cause_r13_2,
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
		"cause-r13"
		},
	{ ATF_POINTER, 2, offsetof(struct WLAN_TargetDeviceErrorCauses_r13, wlan_AP_RSSI_MeasurementNotPossible_r13),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
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
		"wlan-AP-RSSI-MeasurementNotPossible-r13"
		},
	{ ATF_POINTER, 1, offsetof(struct WLAN_TargetDeviceErrorCauses_r13, wlan_AP_RTT_MeasurementNotPossible_r13),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
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
		"wlan-AP-RTT-MeasurementNotPossible-r13"
		},
};
static const int asn_MAP_WLAN_TargetDeviceErrorCauses_r13_oms_1[] = { 1, 2 };
static const ber_tlv_tag_t asn_DEF_WLAN_TargetDeviceErrorCauses_r13_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_WLAN_TargetDeviceErrorCauses_r13_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* cause-r13 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* wlan-AP-RSSI-MeasurementNotPossible-r13 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* wlan-AP-RTT-MeasurementNotPossible-r13 */
};
asn_SEQUENCE_specifics_t asn_SPC_WLAN_TargetDeviceErrorCauses_r13_specs_1 = {
	sizeof(struct WLAN_TargetDeviceErrorCauses_r13),
	offsetof(struct WLAN_TargetDeviceErrorCauses_r13, _asn_ctx),
	asn_MAP_WLAN_TargetDeviceErrorCauses_r13_tag2el_1,
	3,	/* Count of tags in the map */
	asn_MAP_WLAN_TargetDeviceErrorCauses_r13_oms_1,	/* Optional members */
	2, 0,	/* Root/Additions */
	3,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_WLAN_TargetDeviceErrorCauses_r13 = {
	"WLAN-TargetDeviceErrorCauses-r13",
	"WLAN-TargetDeviceErrorCauses-r13",
	&asn_OP_SEQUENCE,
	asn_DEF_WLAN_TargetDeviceErrorCauses_r13_tags_1,
	sizeof(asn_DEF_WLAN_TargetDeviceErrorCauses_r13_tags_1)
		/sizeof(asn_DEF_WLAN_TargetDeviceErrorCauses_r13_tags_1[0]), /* 1 */
	asn_DEF_WLAN_TargetDeviceErrorCauses_r13_tags_1,	/* Same as above */
	sizeof(asn_DEF_WLAN_TargetDeviceErrorCauses_r13_tags_1)
		/sizeof(asn_DEF_WLAN_TargetDeviceErrorCauses_r13_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_WLAN_TargetDeviceErrorCauses_r13_1,
	3,	/* Elements count */
	&asn_SPC_WLAN_TargetDeviceErrorCauses_r13_specs_1	/* Additional specs */
};

