/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "CommonIEsRequestAssistanceData.h"

#include "ECGI.h"
#include "PeriodicAssistanceDataControlParameters-r15.h"
#include "NCGI-r15.h"
static asn_TYPE_member_t asn_MBR_ext1_4[] = {
	{ ATF_POINTER, 1, offsetof(struct CommonIEsRequestAssistanceData__ext1, segmentationInfo_r14),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SegmentationInfo_r14,
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
		"segmentationInfo-r14"
		},
};
static const int asn_MAP_ext1_oms_4[] = { 0 };
static const ber_tlv_tag_t asn_DEF_ext1_tags_4[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext1_tag2el_4[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* segmentationInfo-r14 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext1_specs_4 = {
	sizeof(struct CommonIEsRequestAssistanceData__ext1),
	offsetof(struct CommonIEsRequestAssistanceData__ext1, _asn_ctx),
	asn_MAP_ext1_tag2el_4,
	1,	/* Count of tags in the map */
	asn_MAP_ext1_oms_4,	/* Optional members */
	1, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_ext1_4 = {
	"ext1",
	"ext1",
	&asn_OP_SEQUENCE,
	asn_DEF_ext1_tags_4,
	sizeof(asn_DEF_ext1_tags_4)
		/sizeof(asn_DEF_ext1_tags_4[0]) - 1, /* 1 */
	asn_DEF_ext1_tags_4,	/* Same as above */
	sizeof(asn_DEF_ext1_tags_4)
		/sizeof(asn_DEF_ext1_tags_4[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_ext1_4,
	1,	/* Elements count */
	&asn_SPC_ext1_specs_4	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_ext2_6[] = {
	{ ATF_POINTER, 2, offsetof(struct CommonIEsRequestAssistanceData__ext2, periodicAssistanceDataReq_r15),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PeriodicAssistanceDataControlParameters_r15,
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
		"periodicAssistanceDataReq-r15"
		},
	{ ATF_POINTER, 1, offsetof(struct CommonIEsRequestAssistanceData__ext2, primaryCellID_r15),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
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
		"primaryCellID-r15"
		},
};
static const int asn_MAP_ext2_oms_6[] = { 0, 1 };
static const ber_tlv_tag_t asn_DEF_ext2_tags_6[] = {
	(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ext2_tag2el_6[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* periodicAssistanceDataReq-r15 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* primaryCellID-r15 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ext2_specs_6 = {
	sizeof(struct CommonIEsRequestAssistanceData__ext2),
	offsetof(struct CommonIEsRequestAssistanceData__ext2, _asn_ctx),
	asn_MAP_ext2_tag2el_6,
	2,	/* Count of tags in the map */
	asn_MAP_ext2_oms_6,	/* Optional members */
	2, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_ext2_6 = {
	"ext2",
	"ext2",
	&asn_OP_SEQUENCE,
	asn_DEF_ext2_tags_6,
	sizeof(asn_DEF_ext2_tags_6)
		/sizeof(asn_DEF_ext2_tags_6[0]) - 1, /* 1 */
	asn_DEF_ext2_tags_6,	/* Same as above */
	sizeof(asn_DEF_ext2_tags_6)
		/sizeof(asn_DEF_ext2_tags_6[0]), /* 2 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_ext2_6,
	2,	/* Elements count */
	&asn_SPC_ext2_specs_6	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_CommonIEsRequestAssistanceData_1[] = {
	{ ATF_POINTER, 3, offsetof(struct CommonIEsRequestAssistanceData, primaryCellID),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ECGI,
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
		"primaryCellID"
		},
	{ ATF_POINTER, 2, offsetof(struct CommonIEsRequestAssistanceData, ext1),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		0,
		&asn_DEF_ext1_4,
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
	{ ATF_POINTER, 1, offsetof(struct CommonIEsRequestAssistanceData, ext2),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		0,
		&asn_DEF_ext2_6,
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
static const int asn_MAP_CommonIEsRequestAssistanceData_oms_1[] = { 0, 1, 2 };
static const ber_tlv_tag_t asn_DEF_CommonIEsRequestAssistanceData_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_CommonIEsRequestAssistanceData_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* primaryCellID */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* ext1 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* ext2 */
};
asn_SEQUENCE_specifics_t asn_SPC_CommonIEsRequestAssistanceData_specs_1 = {
	sizeof(struct CommonIEsRequestAssistanceData),
	offsetof(struct CommonIEsRequestAssistanceData, _asn_ctx),
	asn_MAP_CommonIEsRequestAssistanceData_tag2el_1,
	3,	/* Count of tags in the map */
	asn_MAP_CommonIEsRequestAssistanceData_oms_1,	/* Optional members */
	1, 2,	/* Root/Additions */
	1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_CommonIEsRequestAssistanceData = {
	"CommonIEsRequestAssistanceData",
	"CommonIEsRequestAssistanceData",
	&asn_OP_SEQUENCE,
	asn_DEF_CommonIEsRequestAssistanceData_tags_1,
	sizeof(asn_DEF_CommonIEsRequestAssistanceData_tags_1)
		/sizeof(asn_DEF_CommonIEsRequestAssistanceData_tags_1[0]), /* 1 */
	asn_DEF_CommonIEsRequestAssistanceData_tags_1,	/* Same as above */
	sizeof(asn_DEF_CommonIEsRequestAssistanceData_tags_1)
		/sizeof(asn_DEF_CommonIEsRequestAssistanceData_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		SEQUENCE_constraint
	},
	asn_MBR_CommonIEsRequestAssistanceData_1,
	3,	/* Elements count */
	&asn_SPC_CommonIEsRequestAssistanceData_specs_1	/* Additional specs */
};

