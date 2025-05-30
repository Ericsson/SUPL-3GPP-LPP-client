/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#ifndef	_PeriodicReportingIntervalMsSupport_r18_H_
#define	_PeriodicReportingIntervalMsSupport_r18_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum PeriodicReportingIntervalMsSupport_r18__minPeriodicReportingIntervalMs_r18 {
	PeriodicReportingIntervalMsSupport_r18__minPeriodicReportingIntervalMs_r18_ms1	= 0,
	PeriodicReportingIntervalMsSupport_r18__minPeriodicReportingIntervalMs_r18_ms10	= 1,
	PeriodicReportingIntervalMsSupport_r18__minPeriodicReportingIntervalMs_r18_ms100	= 2
	/*
	 * Enumeration is extensible
	 */
} e_PeriodicReportingIntervalMsSupport_r18__minPeriodicReportingIntervalMs_r18;

/* PeriodicReportingIntervalMsSupport-r18 */
typedef struct PeriodicReportingIntervalMsSupport_r18 {
	long	 minPeriodicReportingIntervalMs_r18;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PeriodicReportingIntervalMsSupport_r18_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_minPeriodicReportingIntervalMs_r18_2;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_PeriodicReportingIntervalMsSupport_r18;
extern asn_SEQUENCE_specifics_t asn_SPC_PeriodicReportingIntervalMsSupport_r18_specs_1;
extern asn_TYPE_member_t asn_MBR_PeriodicReportingIntervalMsSupport_r18_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _PeriodicReportingIntervalMsSupport_r18_H_ */
#include <asn_internal.h>
