/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "NR-DL-PRS-ExpectedLOS-NLOS-AssistancePerTRP-r17.h"

#include "NCGI-r15.h"
#include "NR-DL-PRS-ExpectedLOS-NLOS-AssistancePerResource-r17.h"
static int
memb_perResource_r17_constraint_6(const asn_TYPE_descriptor_t *td, const void *sptr,
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
	
	if((size >= 1UL && size <= 2UL)) {
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
memb_dl_PRS_ID_r17_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
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
static asn_per_constraints_t asn_PER_type_perResource_r17_constr_8 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 1,  1,  1,  2 }	/* (SIZE(1..2)) */,
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_perResource_r17_constr_8 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 1,  1,  1,  2 }	/* (SIZE(1..2)) */,
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_type_nr_los_nlos_indicator_r17_constr_6 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_dl_PRS_ID_r17_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 8,  8,  0,  255 }	/* (0..255) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static asn_TYPE_member_t asn_MBR_perResource_r17_8[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerResource_r17,
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
static const ber_tlv_tag_t asn_DEF_perResource_r17_tags_8[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_perResource_r17_specs_8 = {
	sizeof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17__nr_los_nlos_indicator_r17__perResource_r17),
	offsetof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17__nr_los_nlos_indicator_r17__perResource_r17, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_perResource_r17_8 = {
	"perResource-r17",
	"perResource-r17",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_perResource_r17_tags_8,
	sizeof(asn_DEF_perResource_r17_tags_8)
		/sizeof(asn_DEF_perResource_r17_tags_8[0]) - 1, /* 1 */
	asn_DEF_perResource_r17_tags_8,	/* Same as above */
	sizeof(asn_DEF_perResource_r17_tags_8)
		/sizeof(asn_DEF_perResource_r17_tags_8[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_perResource_r17_constr_8,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_OF_constraint
	},
	asn_MBR_perResource_r17_8,
	1,	/* Single element */
	&asn_SPC_perResource_r17_specs_8	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_nr_los_nlos_indicator_r17_6[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17__nr_los_nlos_indicator_r17, choice.perTrp_r17),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LOS_NLOS_Indicator_r17,
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
		"perTrp-r17"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17__nr_los_nlos_indicator_r17, choice.perResource_r17),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		0,
		&asn_DEF_perResource_r17_8,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_perResource_r17_constr_8,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_perResource_r17_constraint_6
		},
		0, 0, /* No default value */
		"perResource-r17"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_nr_los_nlos_indicator_r17_tag2el_6[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* perTrp-r17 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* perResource-r17 */
};
static asn_CHOICE_specifics_t asn_SPC_nr_los_nlos_indicator_r17_specs_6 = {
	sizeof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17__nr_los_nlos_indicator_r17),
	offsetof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17__nr_los_nlos_indicator_r17, _asn_ctx),
	offsetof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17__nr_los_nlos_indicator_r17, present),
	sizeof(((struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17__nr_los_nlos_indicator_r17 *)0)->present),
	asn_MAP_nr_los_nlos_indicator_r17_tag2el_6,
	2,	/* Count of tags in the map */
	0, 0,
	-1	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_nr_los_nlos_indicator_r17_6 = {
	"nr-los-nlos-indicator-r17",
	"nr-los-nlos-indicator-r17",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_nr_los_nlos_indicator_r17_constr_6,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		CHOICE_constraint
	},
	asn_MBR_nr_los_nlos_indicator_r17_6,
	2,	/* Elements count */
	&asn_SPC_nr_los_nlos_indicator_r17_specs_6	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17, dl_PRS_ID_r17),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_dl_PRS_ID_r17_constr_2,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_dl_PRS_ID_r17_constraint_1
		},
		0, 0, /* No default value */
		"dl-PRS-ID-r17"
		},
	{ ATF_POINTER, 3, offsetof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17, nr_PhysCellID_r17),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
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
		"nr-PhysCellID-r17"
		},
	{ ATF_POINTER, 2, offsetof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17, nr_CellGlobalID_r17),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NCGI_r15,
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
		"nr-CellGlobalID-r17"
		},
	{ ATF_POINTER, 1, offsetof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17, nr_ARFCN_r17),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ARFCN_ValueNR_r15,
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
		"nr-ARFCN-r17"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17, nr_los_nlos_indicator_r17),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_nr_los_nlos_indicator_r17_6,
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
		"nr-los-nlos-indicator-r17"
		},
};
static const int asn_MAP_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_oms_1[] = { 1, 2, 3 };
static const ber_tlv_tag_t asn_DEF_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* dl-PRS-ID-r17 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* nr-PhysCellID-r17 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* nr-CellGlobalID-r17 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* nr-ARFCN-r17 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* nr-los-nlos-indicator-r17 */
};
asn_SEQUENCE_specifics_t asn_SPC_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_specs_1 = {
	sizeof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17),
	offsetof(struct NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17, _asn_ctx),
	asn_MAP_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_tag2el_1,
	5,	/* Count of tags in the map */
	asn_MAP_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_oms_1,	/* Optional members */
	3, 0,	/* Root/Additions */
	5,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17 = {
	"NR-DL-PRS-ExpectedLOS-NLOS-AssistancePerTRP-r17",
	"NR-DL-PRS-ExpectedLOS-NLOS-AssistancePerTRP-r17",
	&asn_OP_SEQUENCE,
	asn_DEF_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_tags_1,
	sizeof(asn_DEF_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_tags_1)
		/sizeof(asn_DEF_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_tags_1[0]), /* 1 */
	asn_DEF_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_tags_1,	/* Same as above */
	sizeof(asn_DEF_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_tags_1)
		/sizeof(asn_DEF_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_1,
	5,	/* Elements count */
	&asn_SPC_NR_DL_PRS_ExpectedLOS_NLOS_AssistancePerTRP_r17_specs_1	/* Additional specs */
};

