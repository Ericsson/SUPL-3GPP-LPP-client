/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_GNSS_RTK_FKP_GradientsSupport_r15_H_
#define	_GNSS_RTK_FKP_GradientsSupport_r15_H_


#include <asn_application.h>

/* Including external dependencies */
#include "GNSS-Link-CombinationsList-r15.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GNSS-RTK-FKP-GradientsSupport-r15 */
typedef struct GNSS_RTK_FKP_GradientsSupport_r15 {
	GNSS_Link_CombinationsList_r15_t	 link_combinations_support_r15;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GNSS_RTK_FKP_GradientsSupport_r15_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GNSS_RTK_FKP_GradientsSupport_r15;
extern asn_SEQUENCE_specifics_t asn_SPC_GNSS_RTK_FKP_GradientsSupport_r15_specs_1;
extern asn_TYPE_member_t asn_MBR_GNSS_RTK_FKP_GradientsSupport_r15_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _GNSS_RTK_FKP_GradientsSupport_r15_H_ */
#include <asn_internal.h>
