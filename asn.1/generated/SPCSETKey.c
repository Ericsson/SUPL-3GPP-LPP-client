/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Ver2-ULP-Components"
 * 	found in "/home/martin/repos/LPP-Client/asn/ULP-Components.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#include "SPCSETKey.h"

int
SPCSETKey_constraint(const asn_TYPE_descriptor_t *td, const void *sptr,
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
	
	if((size == 128)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

/*
 * This type is implemented using BIT_STRING,
 * so here we adjust the DEF accordingly.
 */
asn_per_constraints_t asn_PER_type_SPCSETKey_constr_1 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 0,  0,  128,  128 }	/* (SIZE(128..128)) */,
	0, 0	/* No PER value map */
};
static const ber_tlv_tag_t asn_DEF_SPCSETKey_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (3 << 2))
};
asn_TYPE_descriptor_t asn_DEF_SPCSETKey = {
	"SPCSETKey",
	"SPCSETKey",
	&asn_OP_BIT_STRING,
	asn_DEF_SPCSETKey_tags_1,
	sizeof(asn_DEF_SPCSETKey_tags_1)
		/sizeof(asn_DEF_SPCSETKey_tags_1[0]), /* 1 */
	asn_DEF_SPCSETKey_tags_1,	/* Same as above */
	sizeof(asn_DEF_SPCSETKey_tags_1)
		/sizeof(asn_DEF_SPCSETKey_tags_1[0]), /* 1 */
	{ 0, &asn_PER_type_SPCSETKey_constr_1, SPCSETKey_constraint },
	0, 0,	/* No members */
	&asn_SPC_BIT_STRING_specs	/* Additional specs */
};

