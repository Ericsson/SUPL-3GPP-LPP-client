/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "GNSS-ReferenceStationID-r15.h"

static int check_permitted_alphabet_3(const void *sptr) {
	/* The underlying type is VisibleString */
	const VisibleString_t *st = (const VisibleString_t *)sptr;
	const uint8_t *ch = st->buf;
	const uint8_t *end = ch + st->size;
	
	for(; ch < end; ch++) {
		uint8_t cv = *ch;
		if(!(cv >= 32UL && cv <= 126UL)) return -1;
	}
	return 0;
}

static int
memb_referenceStationID_r15_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0L && value <= 65535L)) {
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
memb_providerName_r15_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	const VisibleString_t *st = (const VisibleString_t *)sptr;
	size_t size;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	size = st->size;
	
	if((size >= 1UL && size <= 32UL)
		 && !check_permitted_alphabet_3(st)) {
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
static asn_per_constraints_t asn_PER_memb_referenceStationID_r15_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 16,  16,  0,  65535 }	/* (0..65535) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_providerName_r15_constr_3 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 7,  7,  32,  126 }	/* (32..126) */,
	{ APC_CONSTRAINED,	 5,  5,  1,  32 }	/* (SIZE(1..32)) */,
	0, 0	/* No PER character map necessary */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
asn_TYPE_member_t asn_MBR_GNSS_ReferenceStationID_r15_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct GNSS_ReferenceStationID_r15, referenceStationID_r15),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_referenceStationID_r15_constr_2,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_referenceStationID_r15_constraint_1
		},
		0, 0, /* No default value */
		"referenceStationID-r15"
		},
	{ ATF_POINTER, 1, offsetof(struct GNSS_ReferenceStationID_r15, providerName_r15),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_VisibleString,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_providerName_r15_constr_3,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_providerName_r15_constraint_1
		},
		0, 0, /* No default value */
		"providerName-r15"
		},
};
static const int asn_MAP_GNSS_ReferenceStationID_r15_oms_1[] = { 1 };
static const ber_tlv_tag_t asn_DEF_GNSS_ReferenceStationID_r15_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_GNSS_ReferenceStationID_r15_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* referenceStationID-r15 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* providerName-r15 */
};
asn_SEQUENCE_specifics_t asn_SPC_GNSS_ReferenceStationID_r15_specs_1 = {
	sizeof(struct GNSS_ReferenceStationID_r15),
	offsetof(struct GNSS_ReferenceStationID_r15, _asn_ctx),
	asn_MAP_GNSS_ReferenceStationID_r15_tag2el_1,
	2,	/* Count of tags in the map */
	asn_MAP_GNSS_ReferenceStationID_r15_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	2,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_GNSS_ReferenceStationID_r15 = {
	"GNSS-ReferenceStationID-r15",
	"GNSS-ReferenceStationID-r15",
	&asn_OP_SEQUENCE,
	asn_DEF_GNSS_ReferenceStationID_r15_tags_1,
	sizeof(asn_DEF_GNSS_ReferenceStationID_r15_tags_1)
		/sizeof(asn_DEF_GNSS_ReferenceStationID_r15_tags_1[0]), /* 1 */
	asn_DEF_GNSS_ReferenceStationID_r15_tags_1,	/* Same as above */
	sizeof(asn_DEF_GNSS_ReferenceStationID_r15_tags_1)
		/sizeof(asn_DEF_GNSS_ReferenceStationID_r15_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_GNSS_ReferenceStationID_r15_1,
	2,	/* Elements count */
	&asn_SPC_GNSS_ReferenceStationID_r15_specs_1	/* Additional specs */
};

