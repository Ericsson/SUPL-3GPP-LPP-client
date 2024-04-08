/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_LOS_NLOS_IndicatorType2_r17_H_
#define	_LOS_NLOS_IndicatorType2_r17_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum LOS_NLOS_IndicatorType2_r17 {
	LOS_NLOS_IndicatorType2_r17_hardvalue	= 0,
	LOS_NLOS_IndicatorType2_r17_hardAndsoftvalue	= 1
} e_LOS_NLOS_IndicatorType2_r17;

/* LOS-NLOS-IndicatorType2-r17 */
typedef long	 LOS_NLOS_IndicatorType2_r17_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_LOS_NLOS_IndicatorType2_r17_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_LOS_NLOS_IndicatorType2_r17;
extern const asn_INTEGER_specifics_t asn_SPC_LOS_NLOS_IndicatorType2_r17_specs_1;
asn_struct_free_f LOS_NLOS_IndicatorType2_r17_free;
asn_constr_check_f LOS_NLOS_IndicatorType2_r17_constraint;
xer_type_decoder_f LOS_NLOS_IndicatorType2_r17_decode_xer;
xer_type_encoder_f LOS_NLOS_IndicatorType2_r17_encode_xer;
jer_type_encoder_f LOS_NLOS_IndicatorType2_r17_encode_jer;
per_type_decoder_f LOS_NLOS_IndicatorType2_r17_decode_uper;
per_type_encoder_f LOS_NLOS_IndicatorType2_r17_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _LOS_NLOS_IndicatorType2_r17_H_ */
#include <asn_internal.h>
