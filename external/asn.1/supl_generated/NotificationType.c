/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SUPL-INIT"
 * 	found in "src/SUPL-INIT.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D supl_generated/ -S empty_skeleton/`
 */

#include "NotificationType.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
asn_per_constraints_t asn_PER_type_NotificationType_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  3,  3,  0,  4 }	/* (0..4,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static const asn_INTEGER_enum_map_t asn_MAP_NotificationType_value2enum_1[] = {
	{ 0,	28,	"noNotificationNoVerification" },
	{ 1,	16,	"notificationOnly" },
	{ 2,	35,	"notificationAndVerficationAllowedNA" },
	{ 3,	34,	"notificationAndVerficationDeniedNA" },
	{ 4,	15,	"privacyOverride" }
	/* This list is extensible */
};
static const unsigned int asn_MAP_NotificationType_enum2value_1[] = {
	0,	/* noNotificationNoVerification(0) */
	2,	/* notificationAndVerficationAllowedNA(2) */
	3,	/* notificationAndVerficationDeniedNA(3) */
	1,	/* notificationOnly(1) */
	4	/* privacyOverride(4) */
	/* This list is extensible */
};
const asn_INTEGER_specifics_t asn_SPC_NotificationType_specs_1 = {
	asn_MAP_NotificationType_value2enum_1,	/* "tag" => N; sorted by tag */
	asn_MAP_NotificationType_enum2value_1,	/* N => "tag"; sorted by N */
	5,	/* Number of elements in the maps */
	6,	/* Extensions before this member */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_NotificationType_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_NotificationType = {
	"NotificationType",
	"NotificationType",
	&asn_OP_NativeEnumerated,
	asn_DEF_NotificationType_tags_1,
	sizeof(asn_DEF_NotificationType_tags_1)
		/sizeof(asn_DEF_NotificationType_tags_1[0]), /* 1 */
	asn_DEF_NotificationType_tags_1,	/* Same as above */
	sizeof(asn_DEF_NotificationType_tags_1)
		/sizeof(asn_DEF_NotificationType_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_NotificationType_constr_1,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_NotificationType_specs_1	/* Additional specs */
};
