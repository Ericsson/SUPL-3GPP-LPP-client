/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "NR-DL-PRS-AssistanceData-r16.h"

#include "NR-DL-PRS-AssistanceDataPerFreq-r16.h"
#include "NR-SSB-Config-r16.h"
static int
memb_nr_DL_PRS_AssistanceDataList_r16_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
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
	
	if((size >= 1UL && size <= 4UL)) {
		/* Perform validation of the inner elements */
		return SEQUENCE_OF_constraint(td, sptr, ctfailcb, app_key);
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static int
memb_nr_SSB_Config_r16_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
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
	
	if((size >= 1UL && size <= 256UL)) {
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
static asn_per_constraints_t asn_PER_type_nr_DL_PRS_AssistanceDataList_r16_constr_3 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 2,  2,  1,  4 }	/* (SIZE(1..4)) */,
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_nr_SSB_Config_r16_constr_5 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 8,  8,  1,  256 }	/* (SIZE(1..256)) */,
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_nr_DL_PRS_AssistanceDataList_r16_constr_3 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 2,  2,  1,  4 }	/* (SIZE(1..4)) */,
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_nr_SSB_Config_r16_constr_5 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 8,  8,  1,  256 }	/* (SIZE(1..256)) */,
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static asn_TYPE_member_t asn_MBR_nr_DL_PRS_AssistanceDataList_r16_3[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_NR_DL_PRS_AssistanceDataPerFreq_r16,
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
static const ber_tlv_tag_t asn_DEF_nr_DL_PRS_AssistanceDataList_r16_tags_3[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_nr_DL_PRS_AssistanceDataList_r16_specs_3 = {
	sizeof(struct NR_DL_PRS_AssistanceData_r16__nr_DL_PRS_AssistanceDataList_r16),
	offsetof(struct NR_DL_PRS_AssistanceData_r16__nr_DL_PRS_AssistanceDataList_r16, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_nr_DL_PRS_AssistanceDataList_r16_3 = {
	"nr-DL-PRS-AssistanceDataList-r16",
	"nr-DL-PRS-AssistanceDataList-r16",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_nr_DL_PRS_AssistanceDataList_r16_tags_3,
	sizeof(asn_DEF_nr_DL_PRS_AssistanceDataList_r16_tags_3)
		/sizeof(asn_DEF_nr_DL_PRS_AssistanceDataList_r16_tags_3[0]) - 1, /* 1 */
	asn_DEF_nr_DL_PRS_AssistanceDataList_r16_tags_3,	/* Same as above */
	sizeof(asn_DEF_nr_DL_PRS_AssistanceDataList_r16_tags_3)
		/sizeof(asn_DEF_nr_DL_PRS_AssistanceDataList_r16_tags_3[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_nr_DL_PRS_AssistanceDataList_r16_constr_3,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_OF_constraint
	},
	asn_MBR_nr_DL_PRS_AssistanceDataList_r16_3,
	1,	/* Single element */
	&asn_SPC_nr_DL_PRS_AssistanceDataList_r16_specs_3	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_nr_SSB_Config_r16_5[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_NR_SSB_Config_r16,
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
static const ber_tlv_tag_t asn_DEF_nr_SSB_Config_r16_tags_5[] = {
	(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_nr_SSB_Config_r16_specs_5 = {
	sizeof(struct NR_DL_PRS_AssistanceData_r16__nr_SSB_Config_r16),
	offsetof(struct NR_DL_PRS_AssistanceData_r16__nr_SSB_Config_r16, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_nr_SSB_Config_r16_5 = {
	"nr-SSB-Config-r16",
	"nr-SSB-Config-r16",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_nr_SSB_Config_r16_tags_5,
	sizeof(asn_DEF_nr_SSB_Config_r16_tags_5)
		/sizeof(asn_DEF_nr_SSB_Config_r16_tags_5[0]) - 1, /* 1 */
	asn_DEF_nr_SSB_Config_r16_tags_5,	/* Same as above */
	sizeof(asn_DEF_nr_SSB_Config_r16_tags_5)
		/sizeof(asn_DEF_nr_SSB_Config_r16_tags_5[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_nr_SSB_Config_r16_constr_5,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_OF_constraint
	},
	asn_MBR_nr_SSB_Config_r16_5,
	1,	/* Single element */
	&asn_SPC_nr_SSB_Config_r16_specs_5	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_NR_DL_PRS_AssistanceData_r16_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct NR_DL_PRS_AssistanceData_r16, nr_DL_PRS_ReferenceInfo_r16),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DL_PRS_ID_Info_r16,
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
		"nr-DL-PRS-ReferenceInfo-r16"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct NR_DL_PRS_AssistanceData_r16, nr_DL_PRS_AssistanceDataList_r16),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		0,
		&asn_DEF_nr_DL_PRS_AssistanceDataList_r16_3,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_nr_DL_PRS_AssistanceDataList_r16_constr_3,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_nr_DL_PRS_AssistanceDataList_r16_constraint_1
		},
		0, 0, /* No default value */
		"nr-DL-PRS-AssistanceDataList-r16"
		},
	{ ATF_POINTER, 1, offsetof(struct NR_DL_PRS_AssistanceData_r16, nr_SSB_Config_r16),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		0,
		&asn_DEF_nr_SSB_Config_r16_5,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_nr_SSB_Config_r16_constr_5,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_nr_SSB_Config_r16_constraint_1
		},
		0, 0, /* No default value */
		"nr-SSB-Config-r16"
		},
};
static const int asn_MAP_NR_DL_PRS_AssistanceData_r16_oms_1[] = { 2 };
static const ber_tlv_tag_t asn_DEF_NR_DL_PRS_AssistanceData_r16_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_NR_DL_PRS_AssistanceData_r16_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* nr-DL-PRS-ReferenceInfo-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* nr-DL-PRS-AssistanceDataList-r16 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* nr-SSB-Config-r16 */
};
asn_SEQUENCE_specifics_t asn_SPC_NR_DL_PRS_AssistanceData_r16_specs_1 = {
	sizeof(struct NR_DL_PRS_AssistanceData_r16),
	offsetof(struct NR_DL_PRS_AssistanceData_r16, _asn_ctx),
	asn_MAP_NR_DL_PRS_AssistanceData_r16_tag2el_1,
	3,	/* Count of tags in the map */
	asn_MAP_NR_DL_PRS_AssistanceData_r16_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	3,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_NR_DL_PRS_AssistanceData_r16 = {
	"NR-DL-PRS-AssistanceData-r16",
	"NR-DL-PRS-AssistanceData-r16",
	&asn_OP_SEQUENCE,
	asn_DEF_NR_DL_PRS_AssistanceData_r16_tags_1,
	sizeof(asn_DEF_NR_DL_PRS_AssistanceData_r16_tags_1)
		/sizeof(asn_DEF_NR_DL_PRS_AssistanceData_r16_tags_1[0]), /* 1 */
	asn_DEF_NR_DL_PRS_AssistanceData_r16_tags_1,	/* Same as above */
	sizeof(asn_DEF_NR_DL_PRS_AssistanceData_r16_tags_1)
		/sizeof(asn_DEF_NR_DL_PRS_AssistanceData_r16_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_NR_DL_PRS_AssistanceData_r16_1,
	3,	/* Elements count */
	&asn_SPC_NR_DL_PRS_AssistanceData_r16_specs_1	/* Additional specs */
};

