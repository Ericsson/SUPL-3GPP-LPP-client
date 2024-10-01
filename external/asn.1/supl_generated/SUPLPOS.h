/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SUPL-POS"
 * 	found in "src/SUPL-POS.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D supl_generated/ -S empty_skeleton/`
 */

#ifndef	_SUPLPOS_H_
#define	_SUPLPOS_H_


#include <asn_application.h>

/* Including external dependencies */
#include "PosPayLoad.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ULP_Velocity;
struct Ver2_SUPL_POS_extension;

/* SUPLPOS */
typedef struct SUPLPOS {
	PosPayLoad_t	 posPayLoad;
	struct ULP_Velocity	*velocity;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct Ver2_SUPL_POS_extension	*ver2_SUPL_POS_extension;	/* OPTIONAL */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SUPLPOS_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SUPLPOS;
extern asn_SEQUENCE_specifics_t asn_SPC_SUPLPOS_specs_1;
extern asn_TYPE_member_t asn_MBR_SUPLPOS_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _SUPLPOS_H_ */
#include <asn_internal.h>