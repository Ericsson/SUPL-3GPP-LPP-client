/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "NR-DL-PRS-ResourcesCapability-r16.h"

#include "DL-PRS-ResourcesCapabilityPerBand-r16.h"
/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static int
memb_maxNrOfDL_PRS_ResourceSetPerTrpPerFrequencyLayer_r16_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 1L && value <= 2L)) {
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
memb_maxNrOfPosLayer_r16_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 1L && value <= 4L)) {
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
memb_dl_PRS_ResourcesCapabilityBandList_r16_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	size_t size;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	/* Determine the number of elements */
	size = _A_CSEQUENCE_FROM_VOID(sptr)->count;
	
	if((size >= 1UL && size <= 1024UL)) {
		/* Perform validation of the inner elements */
		return SEQUENCE_OF_constraint(td, sptr, ctfailcb, app_key);
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_maxNrOfTRP_AcrossFreqs_r16_constr_3 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  3,  3,  0,  7 }	/* (0..7,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_dl_PRS_ResourcesCapabilityBandList_r16_constr_15 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 10,  10,  1,  1024 }	/* (SIZE(1..1024)) */,
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_maxNrOfDL_PRS_ResourceSetPerTrpPerFrequencyLayer_r16_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  1,  2 }	/* (1..2) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_maxNrOfPosLayer_r16_constr_14 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 2,  2,  1,  4 }	/* (1..4) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_dl_PRS_ResourcesCapabilityBandList_r16_constr_15 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 10,  10,  1,  1024 }	/* (SIZE(1..1024)) */,
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static const asn_INTEGER_enum_map_t asn_MAP_maxNrOfTRP_AcrossFreqs_r16_value2enum_3[] = {
	{ 0,	2,	"n4" },
	{ 1,	2,	"n6" },
	{ 2,	3,	"n12" },
	{ 3,	3,	"n16" },
	{ 4,	3,	"n32" },
	{ 5,	3,	"n64" },
	{ 6,	4,	"n128" },
	{ 7,	4,	"n256" },
	{ 8,	9,	"n24-v1690" }
	/* This list is extensible */
};
static const unsigned int asn_MAP_maxNrOfTRP_AcrossFreqs_r16_enum2value_3[] = {
	2,	/* n12(2) */
	6,	/* n128(6) */
	3,	/* n16(3) */
	8,	/* n24-v1690(8) */
	7,	/* n256(7) */
	4,	/* n32(4) */
	0,	/* n4(0) */
	1,	/* n6(1) */
	5	/* n64(5) */
	/* This list is extensible */
};
static const asn_INTEGER_specifics_t asn_SPC_maxNrOfTRP_AcrossFreqs_r16_specs_3 = {
	asn_MAP_maxNrOfTRP_AcrossFreqs_r16_value2enum_3,	/* "tag" => N; sorted by tag */
	asn_MAP_maxNrOfTRP_AcrossFreqs_r16_enum2value_3,	/* N => "tag"; sorted by N */
	9,	/* Number of elements in the maps */
	9,	/* Extensions before this member */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_maxNrOfTRP_AcrossFreqs_r16_tags_3[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_maxNrOfTRP_AcrossFreqs_r16_3 = {
	"maxNrOfTRP-AcrossFreqs-r16",
	"maxNrOfTRP-AcrossFreqs-r16",
	&asn_OP_NativeEnumerated,
	asn_DEF_maxNrOfTRP_AcrossFreqs_r16_tags_3,
	sizeof(asn_DEF_maxNrOfTRP_AcrossFreqs_r16_tags_3)
		/sizeof(asn_DEF_maxNrOfTRP_AcrossFreqs_r16_tags_3[0]) - 1, /* 1 */
	asn_DEF_maxNrOfTRP_AcrossFreqs_r16_tags_3,	/* Same as above */
	sizeof(asn_DEF_maxNrOfTRP_AcrossFreqs_r16_tags_3)
		/sizeof(asn_DEF_maxNrOfTRP_AcrossFreqs_r16_tags_3[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_maxNrOfTRP_AcrossFreqs_r16_constr_3,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_maxNrOfTRP_AcrossFreqs_r16_specs_3	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_dl_PRS_ResourcesCapabilityBandList_r16_15[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_DL_PRS_ResourcesCapabilityPerBand_r16,
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
static const ber_tlv_tag_t asn_DEF_dl_PRS_ResourcesCapabilityBandList_r16_tags_15[] = {
	(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_dl_PRS_ResourcesCapabilityBandList_r16_specs_15 = {
	sizeof(struct NR_DL_PRS_ResourcesCapability_r16__dl_PRS_ResourcesCapabilityBandList_r16),
	offsetof(struct NR_DL_PRS_ResourcesCapability_r16__dl_PRS_ResourcesCapabilityBandList_r16, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_dl_PRS_ResourcesCapabilityBandList_r16_15 = {
	"dl-PRS-ResourcesCapabilityBandList-r16",
	"dl-PRS-ResourcesCapabilityBandList-r16",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_dl_PRS_ResourcesCapabilityBandList_r16_tags_15,
	sizeof(asn_DEF_dl_PRS_ResourcesCapabilityBandList_r16_tags_15)
		/sizeof(asn_DEF_dl_PRS_ResourcesCapabilityBandList_r16_tags_15[0]) - 1, /* 1 */
	asn_DEF_dl_PRS_ResourcesCapabilityBandList_r16_tags_15,	/* Same as above */
	sizeof(asn_DEF_dl_PRS_ResourcesCapabilityBandList_r16_tags_15)
		/sizeof(asn_DEF_dl_PRS_ResourcesCapabilityBandList_r16_tags_15[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_dl_PRS_ResourcesCapabilityBandList_r16_constr_15,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_OF_constraint
	},
	asn_MBR_dl_PRS_ResourcesCapabilityBandList_r16_15,
	1,	/* Single element */
	&asn_SPC_dl_PRS_ResourcesCapabilityBandList_r16_specs_15	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_NR_DL_PRS_ResourcesCapability_r16_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct NR_DL_PRS_ResourcesCapability_r16, maxNrOfDL_PRS_ResourceSetPerTrpPerFrequencyLayer_r16),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_maxNrOfDL_PRS_ResourceSetPerTrpPerFrequencyLayer_r16_constr_2,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_maxNrOfDL_PRS_ResourceSetPerTrpPerFrequencyLayer_r16_constraint_1
		},
		0, 0, /* No default value */
		"maxNrOfDL-PRS-ResourceSetPerTrpPerFrequencyLayer-r16"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct NR_DL_PRS_ResourcesCapability_r16, maxNrOfTRP_AcrossFreqs_r16),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_maxNrOfTRP_AcrossFreqs_r16_3,
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
		"maxNrOfTRP-AcrossFreqs-r16"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct NR_DL_PRS_ResourcesCapability_r16, maxNrOfPosLayer_r16),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_maxNrOfPosLayer_r16_constr_14,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_maxNrOfPosLayer_r16_constraint_1
		},
		0, 0, /* No default value */
		"maxNrOfPosLayer-r16"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct NR_DL_PRS_ResourcesCapability_r16, dl_PRS_ResourcesCapabilityBandList_r16),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		0,
		&asn_DEF_dl_PRS_ResourcesCapabilityBandList_r16_15,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_dl_PRS_ResourcesCapabilityBandList_r16_constr_15,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_dl_PRS_ResourcesCapabilityBandList_r16_constraint_1
		},
		0, 0, /* No default value */
		"dl-PRS-ResourcesCapabilityBandList-r16"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct NR_DL_PRS_ResourcesCapability_r16, dl_PRS_ResourcesBandCombinationList_r16),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DL_PRS_ResourcesBandCombinationList_r16,
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
		"dl-PRS-ResourcesBandCombinationList-r16"
		},
};
static const ber_tlv_tag_t asn_DEF_NR_DL_PRS_ResourcesCapability_r16_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_NR_DL_PRS_ResourcesCapability_r16_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* maxNrOfDL-PRS-ResourceSetPerTrpPerFrequencyLayer-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* maxNrOfTRP-AcrossFreqs-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* maxNrOfPosLayer-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* dl-PRS-ResourcesCapabilityBandList-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* dl-PRS-ResourcesBandCombinationList-r16 */
};
asn_SEQUENCE_specifics_t asn_SPC_NR_DL_PRS_ResourcesCapability_r16_specs_1 = {
	sizeof(struct NR_DL_PRS_ResourcesCapability_r16),
	offsetof(struct NR_DL_PRS_ResourcesCapability_r16, _asn_ctx),
	asn_MAP_NR_DL_PRS_ResourcesCapability_r16_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	5,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_NR_DL_PRS_ResourcesCapability_r16 = {
	"NR-DL-PRS-ResourcesCapability-r16",
	"NR-DL-PRS-ResourcesCapability-r16",
	&asn_OP_SEQUENCE,
	asn_DEF_NR_DL_PRS_ResourcesCapability_r16_tags_1,
	sizeof(asn_DEF_NR_DL_PRS_ResourcesCapability_r16_tags_1)
		/sizeof(asn_DEF_NR_DL_PRS_ResourcesCapability_r16_tags_1[0]), /* 1 */
	asn_DEF_NR_DL_PRS_ResourcesCapability_r16_tags_1,	/* Same as above */
	sizeof(asn_DEF_NR_DL_PRS_ResourcesCapability_r16_tags_1)
		/sizeof(asn_DEF_NR_DL_PRS_ResourcesCapability_r16_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_NR_DL_PRS_ResourcesCapability_r16_1,
	5,	/* Elements count */
	&asn_SPC_NR_DL_PRS_ResourcesCapability_r16_specs_1	/* Additional specs */
};

