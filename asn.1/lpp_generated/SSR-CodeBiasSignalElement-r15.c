/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#include "SSR-CodeBiasSignalElement-r15.h"

#include "SSR-IntegrityCodeBiasBounds-r17.h"
static int
memb_codeBias_r15_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -8192L && value <= 8191L)) {
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
static asn_per_constraints_t asn_PER_memb_codeBias_r15_constr_3 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 14,  14, -8192,  8191 }	/* (-8192..8191) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static asn_TYPE_member_t asn_MBR_ext1_5[] = {
	{ ATF_POINTER, 1, offsetof(struct SSR_CodeBiasSignalElement_r15__ext1, ssr_IntegrityCodeBiasBounds_r17),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SSR_IntegrityCodeBiasBounds_r17,
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
		"ssr-IntegrityCodeBiasBounds-r17"
		},
};
static const int asn_MAP_ext1_oms_5[] = { 0 };
static const ber_tlv_tag_t asn_DEF_ext1_tags_5[] = {
	(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext1_tag2el_5[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* ssr-IntegrityCodeBiasBounds-r17 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext1_specs_5 = {
	sizeof(struct SSR_CodeBiasSignalElement_r15__ext1),
	offsetof(struct SSR_CodeBiasSignalElement_r15__ext1, _asn_ctx),
	asn_MAP_ext1_tag2el_5,
	1,	/* Count of tags in the map */
	asn_MAP_ext1_oms_5,	/* Optional members */
	1, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_ext1_5 = {
	"ext1",
	"ext1",
	&asn_OP_SEQUENCE,
	asn_DEF_ext1_tags_5,
	sizeof(asn_DEF_ext1_tags_5)
		/sizeof(asn_DEF_ext1_tags_5[0]) - 1, /* 1 */
	asn_DEF_ext1_tags_5,	/* Same as above */
	sizeof(asn_DEF_ext1_tags_5)
		/sizeof(asn_DEF_ext1_tags_5[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_ext1_5,
	1,	/* Elements count */
	&asn_SPC_ext1_specs_5	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_SSR_CodeBiasSignalElement_r15_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SSR_CodeBiasSignalElement_r15, signal_and_tracking_mode_ID_r15),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GNSS_SignalID,
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
		"signal-and-tracking-mode-ID-r15"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SSR_CodeBiasSignalElement_r15, codeBias_r15),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_codeBias_r15_constr_3,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_codeBias_r15_constraint_1
		},
		0, 0, /* No default value */
		"codeBias-r15"
		},
	{ ATF_POINTER, 1, offsetof(struct SSR_CodeBiasSignalElement_r15, ext1),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		0,
		&asn_DEF_ext1_5,
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
static const int asn_MAP_SSR_CodeBiasSignalElement_r15_oms_1[] = { 2 };
static const ber_tlv_tag_t asn_DEF_SSR_CodeBiasSignalElement_r15_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_SSR_CodeBiasSignalElement_r15_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* signal-and-tracking-mode-ID-r15 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* codeBias-r15 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* ext1 */
};
asn_SEQUENCE_specifics_t asn_SPC_SSR_CodeBiasSignalElement_r15_specs_1 = {
	sizeof(struct SSR_CodeBiasSignalElement_r15),
	offsetof(struct SSR_CodeBiasSignalElement_r15, _asn_ctx),
	asn_MAP_SSR_CodeBiasSignalElement_r15_tag2el_1,
	3,	/* Count of tags in the map */
	asn_MAP_SSR_CodeBiasSignalElement_r15_oms_1,	/* Optional members */
	0, 1,	/* Root/Additions */
	2,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_SSR_CodeBiasSignalElement_r15 = {
	"SSR-CodeBiasSignalElement-r15",
	"SSR-CodeBiasSignalElement-r15",
	&asn_OP_SEQUENCE,
	asn_DEF_SSR_CodeBiasSignalElement_r15_tags_1,
	sizeof(asn_DEF_SSR_CodeBiasSignalElement_r15_tags_1)
		/sizeof(asn_DEF_SSR_CodeBiasSignalElement_r15_tags_1[0]), /* 1 */
	asn_DEF_SSR_CodeBiasSignalElement_r15_tags_1,	/* Same as above */
	sizeof(asn_DEF_SSR_CodeBiasSignalElement_r15_tags_1)
		/sizeof(asn_DEF_SSR_CodeBiasSignalElement_r15_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_SSR_CodeBiasSignalElement_r15_1,
	3,	/* Elements count */
	&asn_SPC_SSR_CodeBiasSignalElement_r15_specs_1	/* Additional specs */
};
