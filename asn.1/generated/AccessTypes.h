/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_AccessTypes_H_
#define	_AccessTypes_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AccessTypes__accessTypes {
	AccessTypes__accessTypes_eutra	= 0,
	AccessTypes__accessTypes_utra	= 1,
	AccessTypes__accessTypes_gsm	= 2,
	AccessTypes__accessTypes_nb_iot	= 3,
	AccessTypes__accessTypes_nr_v1510	= 4
} e_AccessTypes__accessTypes;

/* AccessTypes */
typedef struct AccessTypes {
	BIT_STRING_t	 accessTypes;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AccessTypes_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AccessTypes;
extern asn_SEQUENCE_specifics_t asn_SPC_AccessTypes_specs_1;
extern asn_TYPE_member_t asn_MBR_AccessTypes_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _AccessTypes_H_ */
#include <asn_internal.h>
