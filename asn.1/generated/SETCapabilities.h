/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SUPL-START"
 * 	found in "src/SUPL.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_SETCapabilities_H_
#define	_SETCapabilities_H_


#include <asn_application.h>

/* Including external dependencies */
#include "PosTechnology.h"
#include "PrefMethod.h"
#include "PosProtocol.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Ver2_SETCapabilities_extension;

/* SETCapabilities */
typedef struct SETCapabilities {
	PosTechnology_t	 posTechnology;
	PrefMethod_t	 prefMethod;
	PosProtocol_t	 posProtocol;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct Ver2_SETCapabilities_extension	*ver2_SETCapabilities_extension;	/* OPTIONAL */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SETCapabilities_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SETCapabilities;
extern asn_SEQUENCE_specifics_t asn_SPC_SETCapabilities_specs_1;
extern asn_TYPE_member_t asn_MBR_SETCapabilities_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _SETCapabilities_H_ */
#include <asn_internal.h>
