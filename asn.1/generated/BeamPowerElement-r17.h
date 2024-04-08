/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_BeamPowerElement_r17_H_
#define	_BeamPowerElement_r17_H_


#include <asn_application.h>

/* Including external dependencies */
#include "NR-DL-PRS-ResourceSetID-r16.h"
#include "NR-DL-PRS-ResourceID-r16.h"
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* BeamPowerElement-r17 */
typedef struct BeamPowerElement_r17 {
	NR_DL_PRS_ResourceSetID_r16_t	*nr_dl_prs_ResourceSetID_r17;	/* OPTIONAL */
	NR_DL_PRS_ResourceID_r16_t	 nr_dl_prs_ResourceID_r17;
	long	 nr_dl_prs_RelativePower_r17;
	long	*nr_dl_prs_RelativePowerFine_r17;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} BeamPowerElement_r17_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_BeamPowerElement_r17;
extern asn_SEQUENCE_specifics_t asn_SPC_BeamPowerElement_r17_specs_1;
extern asn_TYPE_member_t asn_MBR_BeamPowerElement_r17_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _BeamPowerElement_r17_H_ */
#include <asn_internal.h>
