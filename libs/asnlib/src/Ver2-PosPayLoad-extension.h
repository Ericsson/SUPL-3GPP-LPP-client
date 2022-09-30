/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ULP-Version-2-parameter-extensions"
 * 	found in "/home/martin/repos/LPP-Client/asn/SUPL.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_Ver2_PosPayLoad_extension_H_
#define	_Ver2_PosPayLoad_extension_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Ver2-PosPayLoad-extension */
typedef struct Ver2_PosPayLoad_extension {
	struct Ver2_PosPayLoad_extension__lPPPayload {
		A_SEQUENCE_OF(OCTET_STRING_t) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *lPPPayload;
	struct Ver2_PosPayLoad_extension__tIA801Payload {
		A_SEQUENCE_OF(OCTET_STRING_t) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *tIA801Payload;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Ver2_PosPayLoad_extension_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ver2_PosPayLoad_extension;
extern asn_SEQUENCE_specifics_t asn_SPC_Ver2_PosPayLoad_extension_specs_1;
extern asn_TYPE_member_t asn_MBR_Ver2_PosPayLoad_extension_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _Ver2_PosPayLoad_extension_H_ */
#include <asn_internal.h>
