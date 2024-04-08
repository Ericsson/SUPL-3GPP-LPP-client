/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "Sensor-MeasurementInformation-r13.h"

static int
memb_range_r14_constraint_6(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0L && value <= 1000L)) {
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
memb_confidence_r14_constraint_6(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 1L && value <= 100L)) {
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
memb_adjustment_r16_constraint_9(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -5000L && value <= 5000L)) {
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
memb_uncompensatedBarometricPressure_r13_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 30000L && value <= 115000L)) {
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
static asn_per_constraints_t asn_PER_memb_range_r14_constr_7 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 10,  10,  0,  1000 }	/* (0..1000) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_confidence_r14_constr_8 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 7,  7,  1,  100 }	/* (1..100) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_adjustment_r16_constr_10 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 14,  14, -5000,  5000 }	/* (-5000..5000) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_uncompensatedBarometricPressure_r13_constr_3 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 17, -1,  30000,  115000 }	/* (30000..115000) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static asn_TYPE_member_t asn_MBR_uncertainty_r14_6[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Sensor_MeasurementInformation_r13__ext1__uncertainty_r14, range_r14),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_range_r14_constr_7,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_range_r14_constraint_6
		},
		0, 0, /* No default value */
		"range-r14"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Sensor_MeasurementInformation_r13__ext1__uncertainty_r14, confidence_r14),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_confidence_r14_constr_8,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_confidence_r14_constraint_6
		},
		0, 0, /* No default value */
		"confidence-r14"
		},
};
static const ber_tlv_tag_t asn_DEF_uncertainty_r14_tags_6[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_uncertainty_r14_tag2el_6[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* range-r14 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* confidence-r14 */
};
static asn_SEQUENCE_specifics_t asn_SPC_uncertainty_r14_specs_6 = {
	sizeof(struct Sensor_MeasurementInformation_r13__ext1__uncertainty_r14),
	offsetof(struct Sensor_MeasurementInformation_r13__ext1__uncertainty_r14, _asn_ctx),
	asn_MAP_uncertainty_r14_tag2el_6,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_uncertainty_r14_6 = {
	"uncertainty-r14",
	"uncertainty-r14",
	&asn_OP_SEQUENCE,
	asn_DEF_uncertainty_r14_tags_6,
	sizeof(asn_DEF_uncertainty_r14_tags_6)
		/sizeof(asn_DEF_uncertainty_r14_tags_6[0]) - 1, /* 1 */
	asn_DEF_uncertainty_r14_tags_6,	/* Same as above */
	sizeof(asn_DEF_uncertainty_r14_tags_6)
		/sizeof(asn_DEF_uncertainty_r14_tags_6[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_uncertainty_r14_6,
	2,	/* Elements count */
	&asn_SPC_uncertainty_r14_specs_6	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_ext1_5[] = {
	{ ATF_POINTER, 1, offsetof(struct Sensor_MeasurementInformation_r13__ext1, uncertainty_r14),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		0,
		&asn_DEF_uncertainty_r14_6,
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
		"uncertainty-r14"
		},
};
static const int asn_MAP_ext1_oms_5[] = { 0 };
static const ber_tlv_tag_t asn_DEF_ext1_tags_5[] = {
	(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext1_tag2el_5[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* uncertainty-r14 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext1_specs_5 = {
	sizeof(struct Sensor_MeasurementInformation_r13__ext1),
	offsetof(struct Sensor_MeasurementInformation_r13__ext1, _asn_ctx),
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

static asn_TYPE_member_t asn_MBR_ext2_9[] = {
	{ ATF_POINTER, 1, offsetof(struct Sensor_MeasurementInformation_r13__ext2, adjustment_r16),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_adjustment_r16_constr_10,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_adjustment_r16_constraint_9
		},
		0, 0, /* No default value */
		"adjustment-r16"
		},
};
static const int asn_MAP_ext2_oms_9[] = { 0 };
static const ber_tlv_tag_t asn_DEF_ext2_tags_9[] = {
	(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext2_tag2el_9[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* adjustment-r16 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext2_specs_9 = {
	sizeof(struct Sensor_MeasurementInformation_r13__ext2),
	offsetof(struct Sensor_MeasurementInformation_r13__ext2, _asn_ctx),
	asn_MAP_ext2_tag2el_9,
	1,	/* Count of tags in the map */
	asn_MAP_ext2_oms_9,	/* Optional members */
	1, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_ext2_9 = {
	"ext2",
	"ext2",
	&asn_OP_SEQUENCE,
	asn_DEF_ext2_tags_9,
	sizeof(asn_DEF_ext2_tags_9)
		/sizeof(asn_DEF_ext2_tags_9[0]) - 1, /* 1 */
	asn_DEF_ext2_tags_9,	/* Same as above */
	sizeof(asn_DEF_ext2_tags_9)
		/sizeof(asn_DEF_ext2_tags_9[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_ext2_9,
	1,	/* Elements count */
	&asn_SPC_ext2_specs_9	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_Sensor_MeasurementInformation_r13_1[] = {
	{ ATF_POINTER, 4, offsetof(struct Sensor_MeasurementInformation_r13, measurementReferenceTime_r13),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_UTCTime,
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
		"measurementReferenceTime-r13"
		},
	{ ATF_POINTER, 3, offsetof(struct Sensor_MeasurementInformation_r13, uncompensatedBarometricPressure_r13),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{
#if !defined(ASN_DISABLE_OER_SUPPORT)
			0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
			&asn_PER_memb_uncompensatedBarometricPressure_r13_constr_3,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
			memb_uncompensatedBarometricPressure_r13_constraint_1
		},
		0, 0, /* No default value */
		"uncompensatedBarometricPressure-r13"
		},
	{ ATF_POINTER, 2, offsetof(struct Sensor_MeasurementInformation_r13, ext1),
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
	{ ATF_POINTER, 1, offsetof(struct Sensor_MeasurementInformation_r13, ext2),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		0,
		&asn_DEF_ext2_9,
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
		"ext2"
		},
};
static const int asn_MAP_Sensor_MeasurementInformation_r13_oms_1[] = { 0, 1, 2, 3 };
static const ber_tlv_tag_t asn_DEF_Sensor_MeasurementInformation_r13_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_Sensor_MeasurementInformation_r13_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* measurementReferenceTime-r13 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* uncompensatedBarometricPressure-r13 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* ext1 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* ext2 */
};
asn_SEQUENCE_specifics_t asn_SPC_Sensor_MeasurementInformation_r13_specs_1 = {
	sizeof(struct Sensor_MeasurementInformation_r13),
	offsetof(struct Sensor_MeasurementInformation_r13, _asn_ctx),
	asn_MAP_Sensor_MeasurementInformation_r13_tag2el_1,
	4,	/* Count of tags in the map */
	asn_MAP_Sensor_MeasurementInformation_r13_oms_1,	/* Optional members */
	2, 2,	/* Root/Additions */
	2,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_Sensor_MeasurementInformation_r13 = {
	"Sensor-MeasurementInformation-r13",
	"Sensor-MeasurementInformation-r13",
	&asn_OP_SEQUENCE,
	asn_DEF_Sensor_MeasurementInformation_r13_tags_1,
	sizeof(asn_DEF_Sensor_MeasurementInformation_r13_tags_1)
		/sizeof(asn_DEF_Sensor_MeasurementInformation_r13_tags_1[0]), /* 1 */
	asn_DEF_Sensor_MeasurementInformation_r13_tags_1,	/* Same as above */
	sizeof(asn_DEF_Sensor_MeasurementInformation_r13_tags_1)
		/sizeof(asn_DEF_Sensor_MeasurementInformation_r13_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_Sensor_MeasurementInformation_r13_1,
	4,	/* Elements count */
	&asn_SPC_Sensor_MeasurementInformation_r13_specs_1	/* Additional specs */
};

