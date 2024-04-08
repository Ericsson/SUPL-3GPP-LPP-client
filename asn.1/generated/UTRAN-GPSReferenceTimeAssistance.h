/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Ver2-ULP-Components"
 * 	found in "src/ULP-Components.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_UTRAN_GPSReferenceTimeAssistance_H_
#define	_UTRAN_GPSReferenceTimeAssistance_H_


#include <asn_application.h>

/* Including external dependencies */
#include "UTRAN-GPSReferenceTime.h"
#include <NativeInteger.h>
#include "UTRANGPSDriftRate.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UTRAN-GPSReferenceTimeAssistance */
typedef struct UTRAN_GPSReferenceTimeAssistance {
	UTRAN_GPSReferenceTime_t	 utran_GPSReferenceTime;
	long	*gpsReferenceTimeUncertainty;	/* OPTIONAL */
	UTRANGPSDriftRate_t	*utranGPSDriftRate;	/* OPTIONAL */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} UTRAN_GPSReferenceTimeAssistance_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UTRAN_GPSReferenceTimeAssistance;
extern asn_SEQUENCE_specifics_t asn_SPC_UTRAN_GPSReferenceTimeAssistance_specs_1;
extern asn_TYPE_member_t asn_MBR_UTRAN_GPSReferenceTimeAssistance_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _UTRAN_GPSReferenceTimeAssistance_H_ */
#include <asn_internal.h>
