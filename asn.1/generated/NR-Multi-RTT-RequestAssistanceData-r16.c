/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "NR-Multi-RTT-RequestAssistanceData-r16.h"

#include "NR-On-Demand-DL-PRS-Request-r17.h"
/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static int
memb_nr_AdType_r16_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	const BIT_STRING_t *st = (const BIT_STRING_t *)sptr;
	size_t size;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	if(st->size > 0) {
		/* Size in bits */
		size = 8 * st->size - (st->bits_unused & 0x07);
	} else {
		size = 0;
	}
	
	if((size >= 1UL && size <= 8UL)) {
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
static asn_per_constraints_t asn_PER_type_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_constr_9 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_pre_configured_AssistanceDataRequest_r17_constr_12 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 0,  0,  0,  0 }	/* (0..0) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_nr_AdType_r16_constr_3 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 3,  3,  1,  8 }	/* (SIZE(1..8)) */,
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static const asn_INTEGER_enum_map_t asn_MAP_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_value2enum_9[] = {
	{ 0,	4,	"eAoD" },
	{ 1,	4,	"eAoA" }
};
static const unsigned int asn_MAP_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_enum2value_9[] = {
	1,	/* eAoA(1) */
	0	/* eAoD(0) */
};
static const asn_INTEGER_specifics_t asn_SPC_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_specs_9 = {
	asn_MAP_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_value2enum_9,	/* "tag" => N; sorted by tag */
	asn_MAP_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_enum2value_9,	/* N => "tag"; sorted by N */
	2,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_tags_9[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_9 = {
	"nr-DL-PRS-ExpectedAoD-or-AoA-Request-r17",
	"nr-DL-PRS-ExpectedAoD-or-AoA-Request-r17",
	&asn_OP_NativeEnumerated,
	asn_DEF_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_tags_9,
	sizeof(asn_DEF_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_tags_9)
		/sizeof(asn_DEF_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_tags_9[0]) - 1, /* 1 */
	asn_DEF_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_tags_9,	/* Same as above */
	sizeof(asn_DEF_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_tags_9)
		/sizeof(asn_DEF_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_tags_9[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_constr_9,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_specs_9	/* Additional specs */
};

static const asn_INTEGER_enum_map_t asn_MAP_pre_configured_AssistanceDataRequest_r17_value2enum_12[] = {
	{ 0,	4,	"true" }
};
static const unsigned int asn_MAP_pre_configured_AssistanceDataRequest_r17_enum2value_12[] = {
	0	/* true(0) */
};
static const asn_INTEGER_specifics_t asn_SPC_pre_configured_AssistanceDataRequest_r17_specs_12 = {
	asn_MAP_pre_configured_AssistanceDataRequest_r17_value2enum_12,	/* "tag" => N; sorted by tag */
	asn_MAP_pre_configured_AssistanceDataRequest_r17_enum2value_12,	/* N => "tag"; sorted by N */
	1,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_pre_configured_AssistanceDataRequest_r17_tags_12[] = {
	(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_pre_configured_AssistanceDataRequest_r17_12 = {
	"pre-configured-AssistanceDataRequest-r17",
	"pre-configured-AssistanceDataRequest-r17",
	&asn_OP_NativeEnumerated,
	asn_DEF_pre_configured_AssistanceDataRequest_r17_tags_12,
	sizeof(asn_DEF_pre_configured_AssistanceDataRequest_r17_tags_12)
		/sizeof(asn_DEF_pre_configured_AssistanceDataRequest_r17_tags_12[0]) - 1, /* 1 */
	asn_DEF_pre_configured_AssistanceDataRequest_r17_tags_12,	/* Same as above */
	sizeof(asn_DEF_pre_configured_AssistanceDataRequest_r17_tags_12)
		/sizeof(asn_DEF_pre_configured_AssistanceDataRequest_r17_tags_12[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_pre_configured_AssistanceDataRequest_r17_constr_12,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_pre_configured_AssistanceDataRequest_r17_specs_12	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_ext1_7[] = {
	{ ATF_POINTER, 3, offsetof(struct NR_Multi_RTT_RequestAssistanceData_r16__ext1, nr_on_demand_DL_PRS_Request_r17),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NR_On_Demand_DL_PRS_Request_r17,
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
		"nr-on-demand-DL-PRS-Request-r17"
		},
	{ ATF_POINTER, 2, offsetof(struct NR_Multi_RTT_RequestAssistanceData_r16__ext1, nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_nr_DL_PRS_ExpectedAoD_or_AoA_Request_r17_9,
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
		"nr-DL-PRS-ExpectedAoD-or-AoA-Request-r17"
		},
	{ ATF_POINTER, 1, offsetof(struct NR_Multi_RTT_RequestAssistanceData_r16__ext1, pre_configured_AssistanceDataRequest_r17),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_pre_configured_AssistanceDataRequest_r17_12,
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
		"pre-configured-AssistanceDataRequest-r17"
		},
};
static const int asn_MAP_ext1_oms_7[] = { 0, 1, 2 };
static const ber_tlv_tag_t asn_DEF_ext1_tags_7[] = {
	(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext1_tag2el_7[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* nr-on-demand-DL-PRS-Request-r17 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* nr-DL-PRS-ExpectedAoD-or-AoA-Request-r17 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* pre-configured-AssistanceDataRequest-r17 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext1_specs_7 = {
	sizeof(struct NR_Multi_RTT_RequestAssistanceData_r16__ext1),
	offsetof(struct NR_Multi_RTT_RequestAssistanceData_r16__ext1, _asn_ctx),
	asn_MAP_ext1_tag2el_7,
	3,	/* Count of tags in the map */
	asn_MAP_ext1_oms_7,	/* Optional members */
	3, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_ext1_7 = {
	"ext1",
	"ext1",
	&asn_OP_SEQUENCE,
	asn_DEF_ext1_tags_7,
	sizeof(asn_DEF_ext1_tags_7)
		/sizeof(asn_DEF_ext1_tags_7[0]) - 1, /* 1 */
	asn_DEF_ext1_tags_7,	/* Same as above */
	sizeof(asn_DEF_ext1_tags_7)
		/sizeof(asn_DEF_ext1_tags_7[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_ext1_7,
	3,	/* Elements count */
	&asn_SPC_ext1_specs_7	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_NR_Multi_RTT_RequestAssistanceData_r16_1[] = {
	{ ATF_POINTER, 1, offsetof(struct NR_Multi_RTT_RequestAssistanceData_r16, nr_PhysCellID_r16),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NR_PhysCellID_r16,
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
		"nr-PhysCellID-r16"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct NR_Multi_RTT_RequestAssistanceData_r16, nr_AdType_r16),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BIT_STRING,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_nr_AdType_r16_constr_3,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_nr_AdType_r16_constraint_1
		},
		0, 0, /* No default value */
		"nr-AdType-r16"
		},
	{ ATF_POINTER, 1, offsetof(struct NR_Multi_RTT_RequestAssistanceData_r16, ext1),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		0,
		&asn_DEF_ext1_7,
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
		"ext1"
		},
};
static const int asn_MAP_NR_Multi_RTT_RequestAssistanceData_r16_oms_1[] = { 0, 2 };
static const ber_tlv_tag_t asn_DEF_NR_Multi_RTT_RequestAssistanceData_r16_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_NR_Multi_RTT_RequestAssistanceData_r16_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* nr-PhysCellID-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* nr-AdType-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* ext1 */
};
asn_SEQUENCE_specifics_t asn_SPC_NR_Multi_RTT_RequestAssistanceData_r16_specs_1 = {
	sizeof(struct NR_Multi_RTT_RequestAssistanceData_r16),
	offsetof(struct NR_Multi_RTT_RequestAssistanceData_r16, _asn_ctx),
	asn_MAP_NR_Multi_RTT_RequestAssistanceData_r16_tag2el_1,
	3,	/* Count of tags in the map */
	asn_MAP_NR_Multi_RTT_RequestAssistanceData_r16_oms_1,	/* Optional members */
	1, 1,	/* Root/Additions */
	2,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_NR_Multi_RTT_RequestAssistanceData_r16 = {
	"NR-Multi-RTT-RequestAssistanceData-r16",
	"NR-Multi-RTT-RequestAssistanceData-r16",
	&asn_OP_SEQUENCE,
	asn_DEF_NR_Multi_RTT_RequestAssistanceData_r16_tags_1,
	sizeof(asn_DEF_NR_Multi_RTT_RequestAssistanceData_r16_tags_1)
		/sizeof(asn_DEF_NR_Multi_RTT_RequestAssistanceData_r16_tags_1[0]), /* 1 */
	asn_DEF_NR_Multi_RTT_RequestAssistanceData_r16_tags_1,	/* Same as above */
	sizeof(asn_DEF_NR_Multi_RTT_RequestAssistanceData_r16_tags_1)
		/sizeof(asn_DEF_NR_Multi_RTT_RequestAssistanceData_r16_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_NR_Multi_RTT_RequestAssistanceData_r16_1,
	3,	/* Elements count */
	&asn_SPC_NR_Multi_RTT_RequestAssistanceData_r16_specs_1	/* Additional specs */
};

