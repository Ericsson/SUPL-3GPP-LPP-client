/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_Sensor_ProvideCapabilities_r13_H_
#define	_Sensor_ProvideCapabilities_r13_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>
#include <NativeEnumerated.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Sensor_ProvideCapabilities_r13__sensor_Modes_r13 {
	Sensor_ProvideCapabilities_r13__sensor_Modes_r13_standalone	= 0,
	Sensor_ProvideCapabilities_r13__sensor_Modes_r13_ue_assisted	= 1,
	Sensor_ProvideCapabilities_r13__sensor_Modes_r13_ue_based	= 2
} e_Sensor_ProvideCapabilities_r13__sensor_Modes_r13;
typedef enum Sensor_ProvideCapabilities_r13__ext1__idleStateForMeasurements_r14 {
	Sensor_ProvideCapabilities_r13__ext1__idleStateForMeasurements_r14_required	= 0
} e_Sensor_ProvideCapabilities_r13__ext1__idleStateForMeasurements_r14;
typedef enum Sensor_ProvideCapabilities_r13__ext2__sensor_MotionInformationSup_r15 {
	Sensor_ProvideCapabilities_r13__ext2__sensor_MotionInformationSup_r15_true	= 0
} e_Sensor_ProvideCapabilities_r13__ext2__sensor_MotionInformationSup_r15;
typedef enum Sensor_ProvideCapabilities_r13__ext3__adjustmentSupported_r16 {
	Sensor_ProvideCapabilities_r13__ext3__adjustmentSupported_r16_true	= 0
} e_Sensor_ProvideCapabilities_r13__ext3__adjustmentSupported_r16;

/* Forward declarations */
struct Sensor_AssistanceDataSupportList_r14;
struct PositioningModes;
struct ScheduledLocationTimeSupportPerMode_r17;

/* Sensor-ProvideCapabilities-r13 */
typedef struct Sensor_ProvideCapabilities_r13 {
	BIT_STRING_t	 sensor_Modes_r13;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct Sensor_ProvideCapabilities_r13__ext1 {
		struct Sensor_AssistanceDataSupportList_r14	*sensor_AssistanceDataSupportList_r14;	/* OPTIONAL */
		struct PositioningModes	*periodicalReportingSupported_r14;	/* OPTIONAL */
		long	*idleStateForMeasurements_r14;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext1;
	struct Sensor_ProvideCapabilities_r13__ext2 {
		long	*sensor_MotionInformationSup_r15;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext2;
	struct Sensor_ProvideCapabilities_r13__ext3 {
		long	*adjustmentSupported_r16;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext3;
	struct Sensor_ProvideCapabilities_r13__ext4 {
		struct ScheduledLocationTimeSupportPerMode_r17	*scheduledLocationRequestSupported_r17;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext4;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Sensor_ProvideCapabilities_r13_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_idleStateForMeasurements_r14_10;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_sensor_MotionInformationSup_r15_13;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_adjustmentSupported_r16_16;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_Sensor_ProvideCapabilities_r13;
extern asn_SEQUENCE_specifics_t asn_SPC_Sensor_ProvideCapabilities_r13_specs_1;
extern asn_TYPE_member_t asn_MBR_Sensor_ProvideCapabilities_r13_1[5];

#ifdef __cplusplus
}
#endif

#endif	/* _Sensor_ProvideCapabilities_r13_H_ */
#include <asn_internal.h>
