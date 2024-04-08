/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Ver2-ULP-Components"
 * 	found in "src/ULP-Components.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_CauseCode_H_
#define	_CauseCode_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CauseCode {
	CauseCode_servingNetWorkNotInAreaIdList	= 0,
	CauseCode_sETCapabilitiesChanged	= 1,
	CauseCode_noSUPLCoverage	= 2
	/*
	 * Enumeration is extensible
	 */
} e_CauseCode;

/* CauseCode */
typedef long	 CauseCode_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_CauseCode_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_CauseCode;
extern const asn_INTEGER_specifics_t asn_SPC_CauseCode_specs_1;
asn_struct_free_f CauseCode_free;
asn_constr_check_f CauseCode_constraint;
xer_type_decoder_f CauseCode_decode_xer;
xer_type_encoder_f CauseCode_encode_xer;
jer_type_encoder_f CauseCode_encode_jer;
per_type_decoder_f CauseCode_decode_uper;
per_type_encoder_f CauseCode_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _CauseCode_H_ */
#include <asn_internal.h>
