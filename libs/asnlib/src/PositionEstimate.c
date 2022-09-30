/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ULP-Components"
 * 	found in "/home/martin/repos/LPP-Client/asn/ULP-Components.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#include "PositionEstimate.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static int
memb_uncertaintySemiMajor_constraint_7(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 127)) {
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
memb_uncertaintySemiMinor_constraint_7(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 127)) {
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
memb_orientationMajorAxis_constraint_7(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 180)) {
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
memb_latitude_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 8388607)) {
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
memb_longitude_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -8388608 && value <= 8388607)) {
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
memb_confidence_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 100)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_per_constraints_t asn_PER_type_latitudeSign_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_uncertaintySemiMajor_constr_8 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 7,  7,  0,  127 }	/* (0..127) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_uncertaintySemiMinor_constr_9 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 7,  7,  0,  127 }	/* (0..127) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_orientationMajorAxis_constr_10 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 8,  8,  0,  180 }	/* (0..180) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_latitude_constr_5 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 23, -1,  0,  8388607 }	/* (0..8388607) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_longitude_constr_6 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 24, -1, -8388608,  8388607 }	/* (-8388608..8388607) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_confidence_constr_11 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 7,  7,  0,  100 }	/* (0..100) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_latitudeSign_value2enum_2[] = {
	{ 0,	5,	"north" },
	{ 1,	5,	"south" }
};
static const unsigned int asn_MAP_latitudeSign_enum2value_2[] = {
	0,	/* north(0) */
	1	/* south(1) */
};
static const asn_INTEGER_specifics_t asn_SPC_latitudeSign_specs_2 = {
	asn_MAP_latitudeSign_value2enum_2,	/* "tag" => N; sorted by tag */
	asn_MAP_latitudeSign_enum2value_2,	/* N => "tag"; sorted by N */
	2,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_latitudeSign_tags_2[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_latitudeSign_2 = {
	"latitudeSign",
	"latitudeSign",
	&asn_OP_NativeEnumerated,
	asn_DEF_latitudeSign_tags_2,
	sizeof(asn_DEF_latitudeSign_tags_2)
		/sizeof(asn_DEF_latitudeSign_tags_2[0]) - 1, /* 1 */
	asn_DEF_latitudeSign_tags_2,	/* Same as above */
	sizeof(asn_DEF_latitudeSign_tags_2)
		/sizeof(asn_DEF_latitudeSign_tags_2[0]), /* 2 */
	{ 0, &asn_PER_type_latitudeSign_constr_2, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_latitudeSign_specs_2	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_uncertainty_7[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PositionEstimate__uncertainty, uncertaintySemiMajor),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_uncertaintySemiMajor_constr_8,  memb_uncertaintySemiMajor_constraint_7 },
		0, 0, /* No default value */
		"uncertaintySemiMajor"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PositionEstimate__uncertainty, uncertaintySemiMinor),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_uncertaintySemiMinor_constr_9,  memb_uncertaintySemiMinor_constraint_7 },
		0, 0, /* No default value */
		"uncertaintySemiMinor"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PositionEstimate__uncertainty, orientationMajorAxis),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_orientationMajorAxis_constr_10,  memb_orientationMajorAxis_constraint_7 },
		0, 0, /* No default value */
		"orientationMajorAxis"
		},
};
static const ber_tlv_tag_t asn_DEF_uncertainty_tags_7[] = {
	(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_uncertainty_tag2el_7[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* uncertaintySemiMajor */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* uncertaintySemiMinor */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* orientationMajorAxis */
};
static asn_SEQUENCE_specifics_t asn_SPC_uncertainty_specs_7 = {
	sizeof(struct PositionEstimate__uncertainty),
	offsetof(struct PositionEstimate__uncertainty, _asn_ctx),
	asn_MAP_uncertainty_tag2el_7,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_uncertainty_7 = {
	"uncertainty",
	"uncertainty",
	&asn_OP_SEQUENCE,
	asn_DEF_uncertainty_tags_7,
	sizeof(asn_DEF_uncertainty_tags_7)
		/sizeof(asn_DEF_uncertainty_tags_7[0]) - 1, /* 1 */
	asn_DEF_uncertainty_tags_7,	/* Same as above */
	sizeof(asn_DEF_uncertainty_tags_7)
		/sizeof(asn_DEF_uncertainty_tags_7[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_uncertainty_7,
	3,	/* Elements count */
	&asn_SPC_uncertainty_specs_7	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_PositionEstimate_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PositionEstimate, latitudeSign),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_latitudeSign_2,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"latitudeSign"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PositionEstimate, latitude),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_latitude_constr_5,  memb_latitude_constraint_1 },
		0, 0, /* No default value */
		"latitude"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PositionEstimate, longitude),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_longitude_constr_6,  memb_longitude_constraint_1 },
		0, 0, /* No default value */
		"longitude"
		},
	{ ATF_POINTER, 3, offsetof(struct PositionEstimate, uncertainty),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		0,
		&asn_DEF_uncertainty_7,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"uncertainty"
		},
	{ ATF_POINTER, 2, offsetof(struct PositionEstimate, confidence),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, &asn_PER_memb_confidence_constr_11,  memb_confidence_constraint_1 },
		0, 0, /* No default value */
		"confidence"
		},
	{ ATF_POINTER, 1, offsetof(struct PositionEstimate, altitudeInfo),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AltitudeInfo,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"altitudeInfo"
		},
};
static const int asn_MAP_PositionEstimate_oms_1[] = { 3, 4, 5 };
static const ber_tlv_tag_t asn_DEF_PositionEstimate_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_PositionEstimate_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* latitudeSign */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* latitude */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* longitude */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* uncertainty */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* confidence */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 } /* altitudeInfo */
};
asn_SEQUENCE_specifics_t asn_SPC_PositionEstimate_specs_1 = {
	sizeof(struct PositionEstimate),
	offsetof(struct PositionEstimate, _asn_ctx),
	asn_MAP_PositionEstimate_tag2el_1,
	6,	/* Count of tags in the map */
	asn_MAP_PositionEstimate_oms_1,	/* Optional members */
	3, 0,	/* Root/Additions */
	6,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_PositionEstimate = {
	"PositionEstimate",
	"PositionEstimate",
	&asn_OP_SEQUENCE,
	asn_DEF_PositionEstimate_tags_1,
	sizeof(asn_DEF_PositionEstimate_tags_1)
		/sizeof(asn_DEF_PositionEstimate_tags_1[0]), /* 1 */
	asn_DEF_PositionEstimate_tags_1,	/* Same as above */
	sizeof(asn_DEF_PositionEstimate_tags_1)
		/sizeof(asn_DEF_PositionEstimate_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_PositionEstimate_1,
	6,	/* Elements count */
	&asn_SPC_PositionEstimate_specs_1	/* Additional specs */
};

