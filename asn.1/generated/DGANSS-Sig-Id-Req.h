/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ULP-Version-2-parameter-extensions"
 * 	found in "src/SUPL.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_DGANSS_Sig_Id_Req_H_
#define	_DGANSS_Sig_Id_Req_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DGANSS-Sig-Id-Req */
typedef BIT_STRING_t	 DGANSS_Sig_Id_Req_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_DGANSS_Sig_Id_Req_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_DGANSS_Sig_Id_Req;
asn_struct_free_f DGANSS_Sig_Id_Req_free;
asn_constr_check_f DGANSS_Sig_Id_Req_constraint;
xer_type_decoder_f DGANSS_Sig_Id_Req_decode_xer;
xer_type_encoder_f DGANSS_Sig_Id_Req_encode_xer;
jer_type_encoder_f DGANSS_Sig_Id_Req_encode_jer;
per_type_decoder_f DGANSS_Sig_Id_Req_decode_uper;
per_type_encoder_f DGANSS_Sig_Id_Req_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _DGANSS_Sig_Id_Req_H_ */
#include <asn_internal.h>
