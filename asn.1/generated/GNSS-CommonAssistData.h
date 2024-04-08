/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_GNSS_CommonAssistData_H_
#define	_GNSS_CommonAssistData_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct GNSS_ReferenceTime;
struct GNSS_ReferenceLocation;
struct GNSS_IonosphericModel;
struct GNSS_EarthOrientationParameters;
struct GNSS_RTK_ReferenceStationInfo_r15;
struct GNSS_RTK_CommonObservationInfo_r15;
struct GNSS_RTK_AuxiliaryStationData_r15;
struct GNSS_SSR_CorrectionPoints_r16;
struct GNSS_Integrity_ServiceParameters_r17;
struct GNSS_Integrity_ServiceAlert_r17;

/* GNSS-CommonAssistData */
typedef struct GNSS_CommonAssistData {
	struct GNSS_ReferenceTime	*gnss_ReferenceTime;	/* OPTIONAL */
	struct GNSS_ReferenceLocation	*gnss_ReferenceLocation;	/* OPTIONAL */
	struct GNSS_IonosphericModel	*gnss_IonosphericModel;	/* OPTIONAL */
	struct GNSS_EarthOrientationParameters	*gnss_EarthOrientationParameters;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct GNSS_CommonAssistData__ext1 {
		struct GNSS_RTK_ReferenceStationInfo_r15	*gnss_RTK_ReferenceStationInfo_r15;	/* OPTIONAL */
		struct GNSS_RTK_CommonObservationInfo_r15	*gnss_RTK_CommonObservationInfo_r15;	/* OPTIONAL */
		struct GNSS_RTK_AuxiliaryStationData_r15	*gnss_RTK_AuxiliaryStationData_r15;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext1;
	struct GNSS_CommonAssistData__ext2 {
		struct GNSS_SSR_CorrectionPoints_r16	*gnss_SSR_CorrectionPoints_r16;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext2;
	struct GNSS_CommonAssistData__ext3 {
		struct GNSS_Integrity_ServiceParameters_r17	*gnss_Integrity_ServiceParameters_r17;	/* OPTIONAL */
		struct GNSS_Integrity_ServiceAlert_r17	*gnss_Integrity_ServiceAlert_r17;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext3;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GNSS_CommonAssistData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GNSS_CommonAssistData;
extern asn_SEQUENCE_specifics_t asn_SPC_GNSS_CommonAssistData_specs_1;
extern asn_TYPE_member_t asn_MBR_GNSS_CommonAssistData_1[7];

#ifdef __cplusplus
}
#endif

#endif	/* _GNSS_CommonAssistData_H_ */
#include <asn_internal.h>
