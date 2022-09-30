/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SUPL-TRIGGERED-START"
 * 	found in "/home/martin/repos/LPP-Client/asn/SUPL.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#include "TriggerParams.h"

asn_per_constraints_t asn_PER_type_TriggerParams_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  1,  1,  0,  1 }	/* (0..1,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
asn_TYPE_member_t asn_MBR_TriggerParams_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct TriggerParams, choice.periodicParams),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PeriodicParams,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"periodicParams"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TriggerParams, choice.areaEventParams),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AreaEventParams,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"areaEventParams"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_TriggerParams_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* periodicParams */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* areaEventParams */
};
asn_CHOICE_specifics_t asn_SPC_TriggerParams_specs_1 = {
	sizeof(struct TriggerParams),
	offsetof(struct TriggerParams, _asn_ctx),
	offsetof(struct TriggerParams, present),
	sizeof(((struct TriggerParams *)0)->present),
	asn_MAP_TriggerParams_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0,
	2	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_TriggerParams = {
	"TriggerParams",
	"TriggerParams",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ 0, &asn_PER_type_TriggerParams_constr_1, CHOICE_constraint },
	asn_MBR_TriggerParams_1,
	2,	/* Elements count */
	&asn_SPC_TriggerParams_specs_1	/* Additional specs */
};

