/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ULP-Components"
 * 	found in "src/ULP-Components.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_Horvel_H_
#define	_Horvel_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Horvel */
typedef struct Horvel {
	BIT_STRING_t	 bearing;
	BIT_STRING_t	 horspeed;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Horvel_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Horvel;
extern asn_SEQUENCE_specifics_t asn_SPC_Horvel_specs_1;
extern asn_TYPE_member_t asn_MBR_Horvel_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _Horvel_H_ */
#include <asn_internal.h>
