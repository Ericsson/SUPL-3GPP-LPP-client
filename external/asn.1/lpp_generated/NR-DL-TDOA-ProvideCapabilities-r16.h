/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#ifndef	_NR_DL_TDOA_ProvideCapabilities_r16_H_
#define	_NR_DL_TDOA_ProvideCapabilities_r16_H_


#include <asn_application.h>

/* Including external dependencies */
#include "PositioningModes.h"
#include "NR-DL-PRS-ResourcesCapability-r16.h"
#include "NR-DL-TDOA-MeasurementCapability-r16.h"
#include "NR-DL-PRS-QCL-ProcessingCapability-r16.h"
#include "NR-DL-PRS-ProcessingCapability-r16.h"
#include <NativeEnumerated.h>
#include <BIT_STRING.h>
#include "LOS-NLOS-IndicatorType2-r17.h"
#include "LOS-NLOS-IndicatorGranularity2-r17.h"
#include <constr_SEQUENCE.h>
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum NR_DL_TDOA_ProvideCapabilities_r16__additionalPathsReport_r16 {
	NR_DL_TDOA_ProvideCapabilities_r16__additionalPathsReport_r16_supported	= 0
} e_NR_DL_TDOA_ProvideCapabilities_r16__additionalPathsReport_r16;
typedef enum NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_PosCalcAssistanceSupport_r17 {
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_PosCalcAssistanceSupport_r17_trpLocSup	= 0,
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_PosCalcAssistanceSupport_r17_beamInfoSup	= 1,
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_PosCalcAssistanceSupport_r17_rtdInfoSup	= 2,
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_PosCalcAssistanceSupport_r17_trpTEG_InfoSup	= 3,
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_PosCalcAssistanceSupport_r17_nr_IntegritySup_r18	= 4,
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_PosCalcAssistanceSupport_r17_pruInfoSup_r18	= 5
} e_NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_PosCalcAssistanceSupport_r17;
typedef enum NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_DL_PRS_ExpectedAoD_or_AoA_Sup_r17 {
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_DL_PRS_ExpectedAoD_or_AoA_Sup_r17_eAoD	= 0,
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_DL_PRS_ExpectedAoD_or_AoA_Sup_r17_eAoA	= 1
} e_NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_DL_PRS_ExpectedAoD_or_AoA_Sup_r17;
typedef enum NR_DL_TDOA_ProvideCapabilities_r16__ext1__additionalPathsExtSupport_r17 {
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__additionalPathsExtSupport_r17_n4	= 0,
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__additionalPathsExtSupport_r17_n6	= 1,
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__additionalPathsExtSupport_r17_n8	= 2
} e_NR_DL_TDOA_ProvideCapabilities_r16__ext1__additionalPathsExtSupport_r17;
typedef enum NR_DL_TDOA_ProvideCapabilities_r16__ext1__multiMeasInSameMeasReport_r17 {
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__multiMeasInSameMeasReport_r17_supported	= 0
} e_NR_DL_TDOA_ProvideCapabilities_r16__ext1__multiMeasInSameMeasReport_r17;
typedef enum NR_DL_TDOA_ProvideCapabilities_r16__ext1__mg_ActivationRequest_r17 {
	NR_DL_TDOA_ProvideCapabilities_r16__ext1__mg_ActivationRequest_r17_supported	= 0
} e_NR_DL_TDOA_ProvideCapabilities_r16__ext1__mg_ActivationRequest_r17;
typedef enum NR_DL_TDOA_ProvideCapabilities_r16__ext2__posMeasGapSupport_r17 {
	NR_DL_TDOA_ProvideCapabilities_r16__ext2__posMeasGapSupport_r17_supported	= 0
} e_NR_DL_TDOA_ProvideCapabilities_r16__ext2__posMeasGapSupport_r17;
typedef enum NR_DL_TDOA_ProvideCapabilities_r16__ext3__multiLocationEstimateInSameMeasReport_r17 {
	NR_DL_TDOA_ProvideCapabilities_r16__ext3__multiLocationEstimateInSameMeasReport_r17_supported	= 0
} e_NR_DL_TDOA_ProvideCapabilities_r16__ext3__multiLocationEstimateInSameMeasReport_r17;
typedef enum NR_DL_TDOA_ProvideCapabilities_r16__ext4__periodicAssistanceData_r18 {
	NR_DL_TDOA_ProvideCapabilities_r16__ext4__periodicAssistanceData_r18_solicited	= 0,
	NR_DL_TDOA_ProvideCapabilities_r16__ext4__periodicAssistanceData_r18_unsolicited	= 1
} e_NR_DL_TDOA_ProvideCapabilities_r16__ext4__periodicAssistanceData_r18;
typedef enum NR_DL_TDOA_ProvideCapabilities_r16__ext4__nr_IntegrityAssistanceSupport_r18 {
	NR_DL_TDOA_ProvideCapabilities_r16__ext4__nr_IntegrityAssistanceSupport_r18_serviceParametersSup	= 0,
	NR_DL_TDOA_ProvideCapabilities_r16__ext4__nr_IntegrityAssistanceSupport_r18_serviceAlertSup	= 1,
	NR_DL_TDOA_ProvideCapabilities_r16__ext4__nr_IntegrityAssistanceSupport_r18_riskParametersSup	= 2,
	NR_DL_TDOA_ProvideCapabilities_r16__ext4__nr_IntegrityAssistanceSupport_r18_integrityParaTRP_LocSup	= 3,
	NR_DL_TDOA_ProvideCapabilities_r16__ext4__nr_IntegrityAssistanceSupport_r18_integrityParaBeamInfoSup	= 4,
	NR_DL_TDOA_ProvideCapabilities_r16__ext4__nr_IntegrityAssistanceSupport_r18_integrityParaRTD_InfoSup	= 5
} e_NR_DL_TDOA_ProvideCapabilities_r16__ext4__nr_IntegrityAssistanceSupport_r18;
typedef enum NR_DL_TDOA_ProvideCapabilities_r16__ext4__nr_DL_TDOA_OnDemandPRS_ForBWA_Support_r18 {
	NR_DL_TDOA_ProvideCapabilities_r16__ext4__nr_DL_TDOA_OnDemandPRS_ForBWA_Support_r18_supported	= 0
} e_NR_DL_TDOA_ProvideCapabilities_r16__ext4__nr_DL_TDOA_OnDemandPRS_ForBWA_Support_r18;

/* Forward declarations */
struct PositioningModes;
struct NR_On_Demand_DL_PRS_Support_r17;
struct ScheduledLocationTimeSupportPerMode_r17;
struct LocationCoordinateTypes;
struct PeriodicReportingIntervalMsSupportPerMode_r18;

/* NR-DL-TDOA-ProvideCapabilities-r16 */
typedef struct NR_DL_TDOA_ProvideCapabilities_r16 {
	PositioningModes_t	 nr_DL_TDOA_Mode_r16;
	NR_DL_PRS_ResourcesCapability_r16_t	 nr_DL_TDOA_PRS_Capability_r16;
	NR_DL_TDOA_MeasurementCapability_r16_t	 nr_DL_TDOA_MeasurementCapability_r16;
	NR_DL_PRS_QCL_ProcessingCapability_r16_t	 nr_DL_PRS_QCL_ProcessingCapability_r16;
	NR_DL_PRS_ProcessingCapability_r16_t	 nr_DL_PRS_ProcessingCapability_r16;
	long	*additionalPathsReport_r16;	/* OPTIONAL */
	struct PositioningModes	*periodicalReporting_r16;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct NR_DL_TDOA_ProvideCapabilities_r16__ext1 {
		struct PositioningModes	*ten_ms_unit_ResponseTime_r17;	/* OPTIONAL */
		BIT_STRING_t	*nr_PosCalcAssistanceSupport_r17;	/* OPTIONAL */
		struct NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_los_nlos_AssistanceDataSupport_r17 {
			LOS_NLOS_IndicatorType2_r17_t	 type_r17;
			LOS_NLOS_IndicatorGranularity2_r17_t	 granularity_r17;
			/*
			 * This type is extensible,
			 * possible extensions are below.
			 */
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} *nr_los_nlos_AssistanceDataSupport_r17;
		BIT_STRING_t	*nr_DL_PRS_ExpectedAoD_or_AoA_Sup_r17;	/* OPTIONAL */
		struct NR_On_Demand_DL_PRS_Support_r17	*nr_DL_TDOA_On_Demand_DL_PRS_Support_r17;	/* OPTIONAL */
		struct NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_los_nlos_IndicatorSupport_r17 {
			LOS_NLOS_IndicatorType2_r17_t	 type_r17;
			LOS_NLOS_IndicatorGranularity2_r17_t	 granularity_r17;
			/*
			 * This type is extensible,
			 * possible extensions are below.
			 */
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} *nr_los_nlos_IndicatorSupport_r17;
		long	*additionalPathsExtSupport_r17;	/* OPTIONAL */
		struct ScheduledLocationTimeSupportPerMode_r17	*scheduledLocationRequestSupported_r17;	/* OPTIONAL */
		struct NR_DL_TDOA_ProvideCapabilities_r16__ext1__nr_dl_prs_AssistanceDataValidity_r17 {
			long	*area_validity_r17;	/* OPTIONAL */
			/*
			 * This type is extensible,
			 * possible extensions are below.
			 */
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} *nr_dl_prs_AssistanceDataValidity_r17;
		long	*multiMeasInSameMeasReport_r17;	/* OPTIONAL */
		long	*mg_ActivationRequest_r17;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext1;
	struct NR_DL_TDOA_ProvideCapabilities_r16__ext2 {
		long	*posMeasGapSupport_r17;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext2;
	struct NR_DL_TDOA_ProvideCapabilities_r16__ext3 {
		long	*multiLocationEstimateInSameMeasReport_r17;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext3;
	struct NR_DL_TDOA_ProvideCapabilities_r16__ext4 {
		struct LocationCoordinateTypes	*locationCoordinateTypes_r18;	/* OPTIONAL */
		BIT_STRING_t	*periodicAssistanceData_r18;	/* OPTIONAL */
		BIT_STRING_t	*nr_IntegrityAssistanceSupport_r18;	/* OPTIONAL */
		long	*nr_DL_TDOA_OnDemandPRS_ForBWA_Support_r18;	/* OPTIONAL */
		struct PeriodicReportingIntervalMsSupportPerMode_r18	*periodicReportingIntervalMsSupport_r18;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext4;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NR_DL_TDOA_ProvideCapabilities_r16_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_additionalPathsReport_r16_7;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_additionalPathsExtSupport_r17_32;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_multiMeasInSameMeasReport_r17_40;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_mg_ActivationRequest_r17_42;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_posMeasGapSupport_r17_45;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_multiLocationEstimateInSameMeasReport_r17_48;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_nr_DL_TDOA_OnDemandPRS_ForBWA_Support_r18_62;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_NR_DL_TDOA_ProvideCapabilities_r16;
extern asn_SEQUENCE_specifics_t asn_SPC_NR_DL_TDOA_ProvideCapabilities_r16_specs_1;
extern asn_TYPE_member_t asn_MBR_NR_DL_TDOA_ProvideCapabilities_r16_1[11];

#ifdef __cplusplus
}
#endif

#endif	/* _NR_DL_TDOA_ProvideCapabilities_r16_H_ */
#include <asn_internal.h>
