/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_Environment_H_
#define	_Environment_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Environment {
	Environment_badArea	= 0,
	Environment_notBadArea	= 1,
	Environment_mixedArea	= 2
	/*
	 * Enumeration is extensible
	 */
} e_Environment;

/* Environment */
typedef long	 Environment_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_Environment_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_Environment;
extern const asn_INTEGER_specifics_t asn_SPC_Environment_specs_1;
asn_struct_free_f Environment_free;
asn_constr_check_f Environment_constraint;
xer_type_decoder_f Environment_decode_xer;
xer_type_encoder_f Environment_encode_xer;
jer_type_encoder_f Environment_encode_jer;
per_type_decoder_f Environment_decode_uper;
per_type_encoder_f Environment_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _Environment_H_ */
#include <asn_internal.h>
