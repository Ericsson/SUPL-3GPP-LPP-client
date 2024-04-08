/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SUPL-TRIGGERED-START"
 * 	found in "src/SUPL.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_TriggerType_H_
#define	_TriggerType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum TriggerType {
	TriggerType_periodic	= 0,
	TriggerType_areaEvent	= 1
	/*
	 * Enumeration is extensible
	 */
} e_TriggerType;

/* TriggerType */
typedef long	 TriggerType_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_TriggerType_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_TriggerType;
extern const asn_INTEGER_specifics_t asn_SPC_TriggerType_specs_1;
asn_struct_free_f TriggerType_free;
asn_constr_check_f TriggerType_constraint;
xer_type_decoder_f TriggerType_decode_xer;
xer_type_encoder_f TriggerType_encode_xer;
jer_type_encoder_f TriggerType_encode_jer;
per_type_decoder_f TriggerType_decode_uper;
per_type_encoder_f TriggerType_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _TriggerType_H_ */
#include <asn_internal.h>
