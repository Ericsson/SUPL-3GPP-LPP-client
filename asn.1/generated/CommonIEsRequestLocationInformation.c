/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "/home/martin/repos/LPP-Client/asn/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#include "CommonIEsRequestLocationInformation.h"

static asn_TYPE_member_t asn_MBR_ext1_11[] = {
	{ ATF_POINTER, 1, offsetof(struct CommonIEsRequestLocationInformation__ext1, messageSizeLimitNB_r14),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MessageSizeLimitNB_r14,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"messageSizeLimitNB-r14"
		},
};
static const int asn_MAP_ext1_oms_11[] = { 0 };
static const ber_tlv_tag_t asn_DEF_ext1_tags_11[] = {
	(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext1_tag2el_11[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* messageSizeLimitNB-r14 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext1_specs_11 = {
	sizeof(struct CommonIEsRequestLocationInformation__ext1),
	offsetof(struct CommonIEsRequestLocationInformation__ext1, _asn_ctx),
	asn_MAP_ext1_tag2el_11,
	1,	/* Count of tags in the map */
	asn_MAP_ext1_oms_11,	/* Optional members */
	1, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_ext1_11 = {
	"ext1",
	"ext1",
	&asn_OP_SEQUENCE,
	asn_DEF_ext1_tags_11,
	sizeof(asn_DEF_ext1_tags_11)
		/sizeof(asn_DEF_ext1_tags_11[0]) - 1, /* 1 */
	asn_DEF_ext1_tags_11,	/* Same as above */
	sizeof(asn_DEF_ext1_tags_11)
		/sizeof(asn_DEF_ext1_tags_11[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_ext1_11,
	1,	/* Elements count */
	&asn_SPC_ext1_specs_11	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_ext2_13[] = {
	{ ATF_POINTER, 1, offsetof(struct CommonIEsRequestLocationInformation__ext2, segmentationInfo_r14),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SegmentationInfo_r14,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"segmentationInfo-r14"
		},
};
static const int asn_MAP_ext2_oms_13[] = { 0 };
static const ber_tlv_tag_t asn_DEF_ext2_tags_13[] = {
	(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext2_tag2el_13[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* segmentationInfo-r14 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext2_specs_13 = {
	sizeof(struct CommonIEsRequestLocationInformation__ext2),
	offsetof(struct CommonIEsRequestLocationInformation__ext2, _asn_ctx),
	asn_MAP_ext2_tag2el_13,
	1,	/* Count of tags in the map */
	asn_MAP_ext2_oms_13,	/* Optional members */
	1, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_ext2_13 = {
	"ext2",
	"ext2",
	&asn_OP_SEQUENCE,
	asn_DEF_ext2_tags_13,
	sizeof(asn_DEF_ext2_tags_13)
		/sizeof(asn_DEF_ext2_tags_13[0]) - 1, /* 1 */
	asn_DEF_ext2_tags_13,	/* Same as above */
	sizeof(asn_DEF_ext2_tags_13)
		/sizeof(asn_DEF_ext2_tags_13[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_ext2_13,
	1,	/* Elements count */
	&asn_SPC_ext2_specs_13	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_CommonIEsRequestLocationInformation_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CommonIEsRequestLocationInformation, locationInformationType),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LocationInformationType,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"locationInformationType"
		},
	{ ATF_POINTER, 9, offsetof(struct CommonIEsRequestLocationInformation, triggeredReporting),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TriggeredReportingCriteria,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"triggeredReporting"
		},
	{ ATF_POINTER, 8, offsetof(struct CommonIEsRequestLocationInformation, periodicalReporting),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PeriodicalReportingCriteria,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"periodicalReporting"
		},
	{ ATF_POINTER, 7, offsetof(struct CommonIEsRequestLocationInformation, additionalInformation),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AdditionalInformation,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"additionalInformation"
		},
	{ ATF_POINTER, 6, offsetof(struct CommonIEsRequestLocationInformation, qos),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_QoS,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"qos"
		},
	{ ATF_POINTER, 5, offsetof(struct CommonIEsRequestLocationInformation, environment),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Environment,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"environment"
		},
	{ ATF_POINTER, 4, offsetof(struct CommonIEsRequestLocationInformation, locationCoordinateTypes),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LocationCoordinateTypes,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"locationCoordinateTypes"
		},
	{ ATF_POINTER, 3, offsetof(struct CommonIEsRequestLocationInformation, velocityTypes),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_VelocityTypes,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"velocityTypes"
		},
	{ ATF_POINTER, 2, offsetof(struct CommonIEsRequestLocationInformation, ext1),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		0,
		&asn_DEF_ext1_11,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ext1"
		},
	{ ATF_POINTER, 1, offsetof(struct CommonIEsRequestLocationInformation, ext2),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		0,
		&asn_DEF_ext2_13,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ext2"
		},
};
static const int asn_MAP_CommonIEsRequestLocationInformation_oms_1[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
static const ber_tlv_tag_t asn_DEF_CommonIEsRequestLocationInformation_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_CommonIEsRequestLocationInformation_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* locationInformationType */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* triggeredReporting */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* periodicalReporting */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* additionalInformation */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* qos */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* environment */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* locationCoordinateTypes */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* velocityTypes */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* ext1 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 } /* ext2 */
};
asn_SEQUENCE_specifics_t asn_SPC_CommonIEsRequestLocationInformation_specs_1 = {
	sizeof(struct CommonIEsRequestLocationInformation),
	offsetof(struct CommonIEsRequestLocationInformation, _asn_ctx),
	asn_MAP_CommonIEsRequestLocationInformation_tag2el_1,
	10,	/* Count of tags in the map */
	asn_MAP_CommonIEsRequestLocationInformation_oms_1,	/* Optional members */
	7, 2,	/* Root/Additions */
	8,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_CommonIEsRequestLocationInformation = {
	"CommonIEsRequestLocationInformation",
	"CommonIEsRequestLocationInformation",
	&asn_OP_SEQUENCE,
	asn_DEF_CommonIEsRequestLocationInformation_tags_1,
	sizeof(asn_DEF_CommonIEsRequestLocationInformation_tags_1)
		/sizeof(asn_DEF_CommonIEsRequestLocationInformation_tags_1[0]), /* 1 */
	asn_DEF_CommonIEsRequestLocationInformation_tags_1,	/* Same as above */
	sizeof(asn_DEF_CommonIEsRequestLocationInformation_tags_1)
		/sizeof(asn_DEF_CommonIEsRequestLocationInformation_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_CommonIEsRequestLocationInformation_1,
	10,	/* Elements count */
	&asn_SPC_CommonIEsRequestLocationInformation_specs_1	/* Additional specs */
};
