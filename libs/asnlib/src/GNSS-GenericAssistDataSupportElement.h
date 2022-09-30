/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "/home/martin/repos/LPP-Client/asn/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_GNSS_GenericAssistDataSupportElement_H_
#define	_GNSS_GenericAssistDataSupportElement_H_


#include <asn_application.h>

/* Including external dependencies */
#include "GNSS-ID.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct SBAS_ID;
struct GNSS_TimeModelListSupport;
struct GNSS_DifferentialCorrectionsSupport;
struct GNSS_NavigationModelSupport;
struct GNSS_RealTimeIntegritySupport;
struct GNSS_DataBitAssistanceSupport;
struct GNSS_AcquisitionAssistanceSupport;
struct GNSS_AlmanacSupport;
struct GNSS_UTC_ModelSupport;
struct GNSS_AuxiliaryInformationSupport;
struct BDS_DifferentialCorrectionsSupport_r12;
struct BDS_GridModelSupport_r12;
struct GNSS_RTK_ObservationsSupport_r15;
struct GLO_RTK_BiasInformationSupport_r15;
struct GNSS_RTK_MAC_CorrectionDifferencesSupport_r15;
struct GNSS_RTK_ResidualsSupport_r15;
struct GNSS_RTK_FKP_GradientsSupport_r15;
struct GNSS_SSR_OrbitCorrectionsSupport_r15;
struct GNSS_SSR_ClockCorrectionsSupport_r15;
struct GNSS_SSR_CodeBiasSupport_r15;
struct GNSS_SSR_URA_Support_r16;
struct GNSS_SSR_PhaseBiasSupport_r16;
struct GNSS_SSR_STEC_CorrectionSupport_r16;
struct GNSS_SSR_GriddedCorrectionSupport_r16;
struct NavIC_DifferentialCorrectionsSupport_r16;
struct NavIC_GridModelSupport_r16;

/* GNSS-GenericAssistDataSupportElement */
typedef struct GNSS_GenericAssistDataSupportElement {
	GNSS_ID_t	 gnss_ID;
	struct SBAS_ID	*sbas_ID	/* OPTIONAL */;
	struct GNSS_TimeModelListSupport	*gnss_TimeModelsSupport	/* OPTIONAL */;
	struct GNSS_DifferentialCorrectionsSupport	*gnss_DifferentialCorrectionsSupport	/* OPTIONAL */;
	struct GNSS_NavigationModelSupport	*gnss_NavigationModelSupport	/* OPTIONAL */;
	struct GNSS_RealTimeIntegritySupport	*gnss_RealTimeIntegritySupport	/* OPTIONAL */;
	struct GNSS_DataBitAssistanceSupport	*gnss_DataBitAssistanceSupport	/* OPTIONAL */;
	struct GNSS_AcquisitionAssistanceSupport	*gnss_AcquisitionAssistanceSupport	/* OPTIONAL */;
	struct GNSS_AlmanacSupport	*gnss_AlmanacSupport	/* OPTIONAL */;
	struct GNSS_UTC_ModelSupport	*gnss_UTC_ModelSupport	/* OPTIONAL */;
	struct GNSS_AuxiliaryInformationSupport	*gnss_AuxiliaryInformationSupport	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct GNSS_GenericAssistDataSupportElement__ext1 {
		struct BDS_DifferentialCorrectionsSupport_r12	*bds_DifferentialCorrectionsSupport_r12	/* OPTIONAL */;
		struct BDS_GridModelSupport_r12	*bds_GridModelSupport_r12	/* OPTIONAL */;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext1;
	struct GNSS_GenericAssistDataSupportElement__ext2 {
		struct GNSS_RTK_ObservationsSupport_r15	*gnss_RTK_ObservationsSupport_r15	/* OPTIONAL */;
		struct GLO_RTK_BiasInformationSupport_r15	*glo_RTK_BiasInformationSupport_r15	/* OPTIONAL */;
		struct GNSS_RTK_MAC_CorrectionDifferencesSupport_r15	*gnss_RTK_MAC_CorrectionDifferencesSupport_r15	/* OPTIONAL */;
		struct GNSS_RTK_ResidualsSupport_r15	*gnss_RTK_ResidualsSupport_r15	/* OPTIONAL */;
		struct GNSS_RTK_FKP_GradientsSupport_r15	*gnss_RTK_FKP_GradientsSupport_r15	/* OPTIONAL */;
		struct GNSS_SSR_OrbitCorrectionsSupport_r15	*gnss_SSR_OrbitCorrectionsSupport_r15	/* OPTIONAL */;
		struct GNSS_SSR_ClockCorrectionsSupport_r15	*gnss_SSR_ClockCorrectionsSupport_r15	/* OPTIONAL */;
		struct GNSS_SSR_CodeBiasSupport_r15	*gnss_SSR_CodeBiasSupport_r15	/* OPTIONAL */;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext2;
	struct GNSS_GenericAssistDataSupportElement__ext3 {
		struct GNSS_SSR_URA_Support_r16	*gnss_SSR_URA_Support_r16	/* OPTIONAL */;
		struct GNSS_SSR_PhaseBiasSupport_r16	*gnss_SSR_PhaseBiasSupport_r16	/* OPTIONAL */;
		struct GNSS_SSR_STEC_CorrectionSupport_r16	*gnss_SSR_STEC_CorrectionSupport_r16	/* OPTIONAL */;
		struct GNSS_SSR_GriddedCorrectionSupport_r16	*gnss_SSR_GriddedCorrectionSupport_r16	/* OPTIONAL */;
		struct NavIC_DifferentialCorrectionsSupport_r16	*navic_DifferentialCorrectionsSupport_r16	/* OPTIONAL */;
		struct NavIC_GridModelSupport_r16	*navic_GridModelSupport_r16	/* OPTIONAL */;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext3;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GNSS_GenericAssistDataSupportElement_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GNSS_GenericAssistDataSupportElement;
extern asn_SEQUENCE_specifics_t asn_SPC_GNSS_GenericAssistDataSupportElement_specs_1;
extern asn_TYPE_member_t asn_MBR_GNSS_GenericAssistDataSupportElement_1[14];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "SBAS-ID.h"
#include "GNSS-TimeModelListSupport.h"
#include "GNSS-DifferentialCorrectionsSupport.h"
#include "GNSS-NavigationModelSupport.h"
#include "GNSS-RealTimeIntegritySupport.h"
#include "GNSS-DataBitAssistanceSupport.h"
#include "GNSS-AcquisitionAssistanceSupport.h"
#include "GNSS-AlmanacSupport.h"
#include "GNSS-UTC-ModelSupport.h"
#include "GNSS-AuxiliaryInformationSupport.h"
#include "BDS-DifferentialCorrectionsSupport-r12.h"
#include "BDS-GridModelSupport-r12.h"
#include "GNSS-RTK-ObservationsSupport-r15.h"
#include "GLO-RTK-BiasInformationSupport-r15.h"
#include "GNSS-RTK-MAC-CorrectionDifferencesSupport-r15.h"
#include "GNSS-RTK-ResidualsSupport-r15.h"
#include "GNSS-RTK-FKP-GradientsSupport-r15.h"
#include "GNSS-SSR-OrbitCorrectionsSupport-r15.h"
#include "GNSS-SSR-ClockCorrectionsSupport-r15.h"
#include "GNSS-SSR-CodeBiasSupport-r15.h"
#include "GNSS-SSR-URA-Support-r16.h"
#include "GNSS-SSR-PhaseBiasSupport-r16.h"
#include "GNSS-SSR-STEC-CorrectionSupport-r16.h"
#include "GNSS-SSR-GriddedCorrectionSupport-r16.h"
#include "NavIC-DifferentialCorrectionsSupport-r16.h"
#include "NavIC-GridModelSupport-r16.h"

#endif	/* _GNSS_GenericAssistDataSupportElement_H_ */
#include <asn_internal.h>
