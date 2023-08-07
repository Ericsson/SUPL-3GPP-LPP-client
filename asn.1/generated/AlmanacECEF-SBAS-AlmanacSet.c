/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "/home/martin/repos/LPP-Client/asn/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#include "AlmanacECEF-SBAS-AlmanacSet.h"

static int
memb_sbasAlmDataID_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 3)) {
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
memb_sbasAlmHealth_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
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
	
	if((size == 8)) {
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
memb_sbasAlmXg_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -16384 && value <= 16383)) {
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
memb_sbasAlmYg_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -16384 && value <= 16383)) {
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
memb_sbasAlmZg_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -256 && value <= 255)) {
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
memb_sbasAlmXgdot_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -4 && value <= 3)) {
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
memb_sbasAlmYgDot_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -4 && value <= 3)) {
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
memb_sbasAlmZgDot_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -8 && value <= 7)) {
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
memb_sbasAlmTo_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 2047)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_per_constraints_t asn_PER_memb_sbasAlmDataID_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 2,  2,  0,  3 }	/* (0..3) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_sbasAlmHealth_constr_4 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 0,  0,  8,  8 }	/* (SIZE(8..8)) */,
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_sbasAlmXg_constr_5 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 15,  15, -16384,  16383 }	/* (-16384..16383) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_sbasAlmYg_constr_6 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 15,  15, -16384,  16383 }	/* (-16384..16383) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_sbasAlmZg_constr_7 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 9,  9, -256,  255 }	/* (-256..255) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_sbasAlmXgdot_constr_8 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 3,  3, -4,  3 }	/* (-4..3) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_sbasAlmYgDot_constr_9 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 3,  3, -4,  3 }	/* (-4..3) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_sbasAlmZgDot_constr_10 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 4,  4, -8,  7 }	/* (-8..7) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_sbasAlmTo_constr_11 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 11,  11,  0,  2047 }	/* (0..2047) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
asn_TYPE_member_t asn_MBR_AlmanacECEF_SBAS_AlmanacSet_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct AlmanacECEF_SBAS_AlmanacSet, sbasAlmDataID),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_sbasAlmDataID_constr_2,  memb_sbasAlmDataID_constraint_1 },
		0, 0, /* No default value */
		"sbasAlmDataID"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AlmanacECEF_SBAS_AlmanacSet, svID),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SV_ID,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"svID"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AlmanacECEF_SBAS_AlmanacSet, sbasAlmHealth),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BIT_STRING,
		0,
		{ 0, &asn_PER_memb_sbasAlmHealth_constr_4,  memb_sbasAlmHealth_constraint_1 },
		0, 0, /* No default value */
		"sbasAlmHealth"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AlmanacECEF_SBAS_AlmanacSet, sbasAlmXg),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_sbasAlmXg_constr_5,  memb_sbasAlmXg_constraint_1 },
		0, 0, /* No default value */
		"sbasAlmXg"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AlmanacECEF_SBAS_AlmanacSet, sbasAlmYg),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_sbasAlmYg_constr_6,  memb_sbasAlmYg_constraint_1 },
		0, 0, /* No default value */
		"sbasAlmYg"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AlmanacECEF_SBAS_AlmanacSet, sbasAlmZg),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_sbasAlmZg_constr_7,  memb_sbasAlmZg_constraint_1 },
		0, 0, /* No default value */
		"sbasAlmZg"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AlmanacECEF_SBAS_AlmanacSet, sbasAlmXgdot),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_sbasAlmXgdot_constr_8,  memb_sbasAlmXgdot_constraint_1 },
		0, 0, /* No default value */
		"sbasAlmXgdot"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AlmanacECEF_SBAS_AlmanacSet, sbasAlmYgDot),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_sbasAlmYgDot_constr_9,  memb_sbasAlmYgDot_constraint_1 },
		0, 0, /* No default value */
		"sbasAlmYgDot"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AlmanacECEF_SBAS_AlmanacSet, sbasAlmZgDot),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_sbasAlmZgDot_constr_10,  memb_sbasAlmZgDot_constraint_1 },
		0, 0, /* No default value */
		"sbasAlmZgDot"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AlmanacECEF_SBAS_AlmanacSet, sbasAlmTo),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_sbasAlmTo_constr_11,  memb_sbasAlmTo_constraint_1 },
		0, 0, /* No default value */
		"sbasAlmTo"
		},
};
static const ber_tlv_tag_t asn_DEF_AlmanacECEF_SBAS_AlmanacSet_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_AlmanacECEF_SBAS_AlmanacSet_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* sbasAlmDataID */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* svID */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* sbasAlmHealth */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* sbasAlmXg */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* sbasAlmYg */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* sbasAlmZg */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* sbasAlmXgdot */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* sbasAlmYgDot */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* sbasAlmZgDot */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 } /* sbasAlmTo */
};
asn_SEQUENCE_specifics_t asn_SPC_AlmanacECEF_SBAS_AlmanacSet_specs_1 = {
	sizeof(struct AlmanacECEF_SBAS_AlmanacSet),
	offsetof(struct AlmanacECEF_SBAS_AlmanacSet, _asn_ctx),
	asn_MAP_AlmanacECEF_SBAS_AlmanacSet_tag2el_1,
	10,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	10,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_AlmanacECEF_SBAS_AlmanacSet = {
	"AlmanacECEF-SBAS-AlmanacSet",
	"AlmanacECEF-SBAS-AlmanacSet",
	&asn_OP_SEQUENCE,
	asn_DEF_AlmanacECEF_SBAS_AlmanacSet_tags_1,
	sizeof(asn_DEF_AlmanacECEF_SBAS_AlmanacSet_tags_1)
		/sizeof(asn_DEF_AlmanacECEF_SBAS_AlmanacSet_tags_1[0]), /* 1 */
	asn_DEF_AlmanacECEF_SBAS_AlmanacSet_tags_1,	/* Same as above */
	sizeof(asn_DEF_AlmanacECEF_SBAS_AlmanacSet_tags_1)
		/sizeof(asn_DEF_AlmanacECEF_SBAS_AlmanacSet_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_AlmanacECEF_SBAS_AlmanacSet_1,
	10,	/* Elements count */
	&asn_SPC_AlmanacECEF_SBAS_AlmanacSet_specs_1	/* Additional specs */
};

