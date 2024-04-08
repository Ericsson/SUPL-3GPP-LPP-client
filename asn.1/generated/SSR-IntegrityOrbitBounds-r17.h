/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_SSR_IntegrityOrbitBounds_r17_H_
#define	_SSR_IntegrityOrbitBounds_r17_H_


#include <asn_application.h>

/* Including external dependencies */
#include "RAC-OrbitalErrorComponents-r17.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SSR-IntegrityOrbitBounds-r17 */
typedef struct SSR_IntegrityOrbitBounds_r17 {
	RAC_OrbitalErrorComponents_r17_t	 meanOrbitError_r17;
	RAC_OrbitalErrorComponents_r17_t	 stdDevOrbitError_r17;
	RAC_OrbitalErrorComponents_r17_t	 meanOrbitRateError_r17;
	RAC_OrbitalErrorComponents_r17_t	 stdDevOrbitRateError_r17;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SSR_IntegrityOrbitBounds_r17_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SSR_IntegrityOrbitBounds_r17;
extern asn_SEQUENCE_specifics_t asn_SPC_SSR_IntegrityOrbitBounds_r17_specs_1;
extern asn_TYPE_member_t asn_MBR_SSR_IntegrityOrbitBounds_r17_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _SSR_IntegrityOrbitBounds_r17_H_ */
#include <asn_internal.h>
