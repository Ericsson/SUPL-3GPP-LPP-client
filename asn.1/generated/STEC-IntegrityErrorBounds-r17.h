/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_STEC_IntegrityErrorBounds_r17_H_
#define	_STEC_IntegrityErrorBounds_r17_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* STEC-IntegrityErrorBounds-r17 */
typedef struct STEC_IntegrityErrorBounds_r17 {
	long	 meanIonosphere_r17;
	long	 stdDevIonosphere_r17;
	long	 meanIonosphereRate_r17;
	long	 stdDevIonosphereRate_r17;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} STEC_IntegrityErrorBounds_r17_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_STEC_IntegrityErrorBounds_r17;
extern asn_SEQUENCE_specifics_t asn_SPC_STEC_IntegrityErrorBounds_r17_specs_1;
extern asn_TYPE_member_t asn_MBR_STEC_IntegrityErrorBounds_r17_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _STEC_IntegrityErrorBounds_r17_H_ */
#include <asn_internal.h>
