/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#include "WLAN-RTT-r13.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static int
memb_rttValue_r13_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0L && value <= 16777215L)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static int
memb_rttAccuracy_r13_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0L && value <= 255L)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_rttUnits_r13_constr_3 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  3,  3,  0,  4 }	/* (0..4,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_rttValue_r13_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 24, -1,  0,  16777215 }	/* (0..16777215) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_rttAccuracy_r13_constr_10 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 8,  8,  0,  255 }	/* (0..255) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static const asn_INTEGER_enum_map_t asn_MAP_rttUnits_r13_value2enum_3[] = {
	{ 0,	12,	"microseconds" },
	{ 1,	21,	"hundredsofnanoseconds" },
	{ 2,	17,	"tensofnanoseconds" },
	{ 3,	11,	"nanoseconds" },
	{ 4,	19,	"tenthsofnanoseconds" }
	/* This list is extensible */
};
static const unsigned int asn_MAP_rttUnits_r13_enum2value_3[] = {
	1,	/* hundredsofnanoseconds(1) */
	0,	/* microseconds(0) */
	3,	/* nanoseconds(3) */
	2,	/* tensofnanoseconds(2) */
	4	/* tenthsofnanoseconds(4) */
	/* This list is extensible */
};
static const asn_INTEGER_specifics_t asn_SPC_rttUnits_r13_specs_3 = {
	asn_MAP_rttUnits_r13_value2enum_3,	/* "tag" => N; sorted by tag */
	asn_MAP_rttUnits_r13_enum2value_3,	/* N => "tag"; sorted by N */
	5,	/* Number of elements in the maps */
	6,	/* Extensions before this member */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_rttUnits_r13_tags_3[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_rttUnits_r13_3 = {
	"rttUnits-r13",
	"rttUnits-r13",
	&asn_OP_NativeEnumerated,
	asn_DEF_rttUnits_r13_tags_3,
	sizeof(asn_DEF_rttUnits_r13_tags_3)
		/sizeof(asn_DEF_rttUnits_r13_tags_3[0]) - 1, /* 1 */
	asn_DEF_rttUnits_r13_tags_3,	/* Same as above */
	sizeof(asn_DEF_rttUnits_r13_tags_3)
		/sizeof(asn_DEF_rttUnits_r13_tags_3[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_rttUnits_r13_constr_3,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_rttUnits_r13_specs_3	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_WLAN_RTT_r13_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct WLAN_RTT_r13, rttValue_r13),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_rttValue_r13_constr_2,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_rttValue_r13_constraint_1
		},
		0, 0, /* No default value */
		"rttValue-r13"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct WLAN_RTT_r13, rttUnits_r13),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_rttUnits_r13_3,
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
		"rttUnits-r13"
		},
	{ ATF_POINTER, 1, offsetof(struct WLAN_RTT_r13, rttAccuracy_r13),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_rttAccuracy_r13_constr_10,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_rttAccuracy_r13_constraint_1
		},
		0, 0, /* No default value */
		"rttAccuracy-r13"
		},
};
static const int asn_MAP_WLAN_RTT_r13_oms_1[] = { 2 };
static const ber_tlv_tag_t asn_DEF_WLAN_RTT_r13_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_WLAN_RTT_r13_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* rttValue-r13 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* rttUnits-r13 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* rttAccuracy-r13 */
};
asn_SEQUENCE_specifics_t asn_SPC_WLAN_RTT_r13_specs_1 = {
	sizeof(struct WLAN_RTT_r13),
	offsetof(struct WLAN_RTT_r13, _asn_ctx),
	asn_MAP_WLAN_RTT_r13_tag2el_1,
	3,	/* Count of tags in the map */
	asn_MAP_WLAN_RTT_r13_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	3,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_WLAN_RTT_r13 = {
	"WLAN-RTT-r13",
	"WLAN-RTT-r13",
	&asn_OP_SEQUENCE,
	asn_DEF_WLAN_RTT_r13_tags_1,
	sizeof(asn_DEF_WLAN_RTT_r13_tags_1)
		/sizeof(asn_DEF_WLAN_RTT_r13_tags_1[0]), /* 1 */
	asn_DEF_WLAN_RTT_r13_tags_1,	/* Same as above */
	sizeof(asn_DEF_WLAN_RTT_r13_tags_1)
		/sizeof(asn_DEF_WLAN_RTT_r13_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_WLAN_RTT_r13_1,
	3,	/* Elements count */
	&asn_SPC_WLAN_RTT_r13_specs_1	/* Additional specs */
};
