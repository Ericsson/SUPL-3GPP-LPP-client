/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#ifndef	_SBAS_ID_H_
#define	_SBAS_ID_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SBAS_ID__sbas_id {
	SBAS_ID__sbas_id_waas	= 0,
	SBAS_ID__sbas_id_egnos	= 1,
	SBAS_ID__sbas_id_msas	= 2,
	SBAS_ID__sbas_id_gagan	= 3
	/*
	 * Enumeration is extensible
	 */
} e_SBAS_ID__sbas_id;

/* SBAS-ID */
typedef struct SBAS_ID {
	long	 sbas_id;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBAS_ID_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_sbas_id_2;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_SBAS_ID;
extern asn_SEQUENCE_specifics_t asn_SPC_SBAS_ID_specs_1;
extern asn_TYPE_member_t asn_MBR_SBAS_ID_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _SBAS_ID_H_ */
#include <asn_internal.h>