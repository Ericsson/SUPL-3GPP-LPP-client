/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_WLAN_RTT_r13_H_
#define	_WLAN_RTT_r13_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <NativeEnumerated.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum WLAN_RTT_r13__rttUnits_r13 {
	WLAN_RTT_r13__rttUnits_r13_microseconds	= 0,
	WLAN_RTT_r13__rttUnits_r13_hundredsofnanoseconds	= 1,
	WLAN_RTT_r13__rttUnits_r13_tensofnanoseconds	= 2,
	WLAN_RTT_r13__rttUnits_r13_nanoseconds	= 3,
	WLAN_RTT_r13__rttUnits_r13_tenthsofnanoseconds	= 4
	/*
	 * Enumeration is extensible
	 */
} e_WLAN_RTT_r13__rttUnits_r13;

/* WLAN-RTT-r13 */
typedef struct WLAN_RTT_r13 {
	long	 rttValue_r13;
	long	 rttUnits_r13;
	long	*rttAccuracy_r13;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} WLAN_RTT_r13_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_rttUnits_r13_3;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_WLAN_RTT_r13;
extern asn_SEQUENCE_specifics_t asn_SPC_WLAN_RTT_r13_specs_1;
extern asn_TYPE_member_t asn_MBR_WLAN_RTT_r13_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _WLAN_RTT_r13_H_ */
#include <asn_internal.h>
