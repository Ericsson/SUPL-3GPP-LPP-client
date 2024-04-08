/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_NR_DL_TDOA_LocationServerErrorCauses_r16_H_
#define	_NR_DL_TDOA_LocationServerErrorCauses_r16_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum NR_DL_TDOA_LocationServerErrorCauses_r16__cause_r16 {
	NR_DL_TDOA_LocationServerErrorCauses_r16__cause_r16_undefined	= 0,
	NR_DL_TDOA_LocationServerErrorCauses_r16__cause_r16_assistanceDataNotSupportedByServer	= 1,
	NR_DL_TDOA_LocationServerErrorCauses_r16__cause_r16_assistanceDataSupportedButCurrentlyNotAvailableByServer	= 2,
	NR_DL_TDOA_LocationServerErrorCauses_r16__cause_r16_notProvidedAssistanceDataNotSupportedByServer	= 3,
	/*
	 * Enumeration is extensible
	 */
	NR_DL_TDOA_LocationServerErrorCauses_r16__cause_r16_on_demand_dl_prs_NotSupportedByServer_v1700	= 4,
	NR_DL_TDOA_LocationServerErrorCauses_r16__cause_r16_on_demand_dl_prs_SupportedButCurrentlyNotAvailableByServer_v1700	= 5
} e_NR_DL_TDOA_LocationServerErrorCauses_r16__cause_r16;

/* NR-DL-TDOA-LocationServerErrorCauses-r16 */
typedef struct NR_DL_TDOA_LocationServerErrorCauses_r16 {
	long	 cause_r16;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NR_DL_TDOA_LocationServerErrorCauses_r16_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_cause_r16_2;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_NR_DL_TDOA_LocationServerErrorCauses_r16;
extern asn_SEQUENCE_specifics_t asn_SPC_NR_DL_TDOA_LocationServerErrorCauses_r16_specs_1;
extern asn_TYPE_member_t asn_MBR_NR_DL_TDOA_LocationServerErrorCauses_r16_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _NR_DL_TDOA_LocationServerErrorCauses_r16_H_ */
#include <asn_internal.h>
