/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_NR_SRS_TxTEG_Element_r17_H_
#define	_NR_SRS_TxTEG_Element_r17_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include "ARFCN-ValueNR-r15.h"
#include <constr_SEQUENCE.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct NR_TimeStamp_r16;

/* NR-SRS-TxTEG-Element-r17 */
typedef struct NR_SRS_TxTEG_Element_r17 {
	struct NR_TimeStamp_r16	*nr_TimeStamp_r17;	/* OPTIONAL */
	long	 nr_UE_Tx_TEG_ID_r17;
	struct NR_SRS_TxTEG_Element_r17__carrierFreq_r17 {
		ARFCN_ValueNR_r15_t	 absoluteFrequencyPointA_r17;
		long	 offsetToPointA_r17;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *carrierFreq_r17;
	struct NR_SRS_TxTEG_Element_r17__srs_PosResourceList_r17 {
		A_SEQUENCE_OF(long) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} srs_PosResourceList_r17;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NR_SRS_TxTEG_Element_r17_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NR_SRS_TxTEG_Element_r17;
extern asn_SEQUENCE_specifics_t asn_SPC_NR_SRS_TxTEG_Element_r17_specs_1;
extern asn_TYPE_member_t asn_MBR_NR_SRS_TxTEG_Element_r17_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _NR_SRS_TxTEG_Element_r17_H_ */
#include <asn_internal.h>
