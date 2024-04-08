/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ULP-Components"
 * 	found in "src/ULP-Components.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#include "PosMethod.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
asn_per_constraints_t asn_PER_type_PosMethod_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  4,  4,  0,  9 }	/* (0..9,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
static const asn_INTEGER_enum_map_t asn_MAP_PosMethod_value2enum_1[] = {
	{ 0,	15,	"agpsSETassisted" },
	{ 1,	12,	"agpsSETbased" },
	{ 2,	19,	"agpsSETassistedpref" },
	{ 3,	16,	"agpsSETbasedpref" },
	{ 4,	13,	"autonomousGPS" },
	{ 5,	4,	"aFLT" },
	{ 6,	4,	"eCID" },
	{ 7,	4,	"eOTD" },
	{ 8,	5,	"oTDOA" },
	{ 9,	10,	"noPosition" },
	{ 10,	28,	"ver2-historicalDataRetrieval" },
	{ 11,	21,	"ver2-agnssSETassisted" },
	{ 12,	18,	"ver2-agnssSETbased" },
	{ 13,	25,	"ver2-agnssSETassistedpref" },
	{ 14,	22,	"ver2-agnssSETbasedpref" },
	{ 15,	19,	"ver2-autonomousGNSS" },
	{ 16,	21,	"ver2-sessioninfoquery" }
	/* This list is extensible */
};
static const unsigned int asn_MAP_PosMethod_enum2value_1[] = {
	5,	/* aFLT(5) */
	0,	/* agpsSETassisted(0) */
	2,	/* agpsSETassistedpref(2) */
	1,	/* agpsSETbased(1) */
	3,	/* agpsSETbasedpref(3) */
	4,	/* autonomousGPS(4) */
	6,	/* eCID(6) */
	7,	/* eOTD(7) */
	9,	/* noPosition(9) */
	8,	/* oTDOA(8) */
	11,	/* ver2-agnssSETassisted(11) */
	13,	/* ver2-agnssSETassistedpref(13) */
	12,	/* ver2-agnssSETbased(12) */
	14,	/* ver2-agnssSETbasedpref(14) */
	15,	/* ver2-autonomousGNSS(15) */
	10,	/* ver2-historicalDataRetrieval(10) */
	16	/* ver2-sessioninfoquery(16) */
	/* This list is extensible */
};
const asn_INTEGER_specifics_t asn_SPC_PosMethod_specs_1 = {
	asn_MAP_PosMethod_value2enum_1,	/* "tag" => N; sorted by tag */
	asn_MAP_PosMethod_enum2value_1,	/* N => "tag"; sorted by N */
	17,	/* Number of elements in the maps */
	11,	/* Extensions before this member */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_PosMethod_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_PosMethod = {
	"PosMethod",
	"PosMethod",
	&asn_OP_NativeEnumerated,
	asn_DEF_PosMethod_tags_1,
	sizeof(asn_DEF_PosMethod_tags_1)
		/sizeof(asn_DEF_PosMethod_tags_1[0]), /* 1 */
	asn_DEF_PosMethod_tags_1,	/* Same as above */
	sizeof(asn_DEF_PosMethod_tags_1)
		/sizeof(asn_DEF_PosMethod_tags_1[0]), /* 1 */
	{
#if !defined(ASN_DISABLE_OER_SUPPORT)
		0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
		&asn_PER_type_PosMethod_constr_1,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
		NativeEnumerated_constraint
	},
	0, 0,	/* Defined elsewhere */
	&asn_SPC_PosMethod_specs_1	/* Additional specs */
};

