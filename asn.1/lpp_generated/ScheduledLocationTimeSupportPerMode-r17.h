/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP-PDU-Definitions.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D lpp_generated/ -S empty_skeleton/`
 */

#ifndef	_ScheduledLocationTimeSupportPerMode_r17_H_
#define	_ScheduledLocationTimeSupportPerMode_r17_H_


#include <asn_application.h>

/* Including external dependencies */
#include "PositioningModes.h"
#include "GNSS-ID-Bitmap.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct PositioningModes;

/* ScheduledLocationTimeSupportPerMode-r17 */
typedef struct ScheduledLocationTimeSupportPerMode_r17 {
	struct PositioningModes	*utcTime_r17;	/* OPTIONAL */
	struct ScheduledLocationTimeSupportPerMode_r17__gnssTime_r17 {
		PositioningModes_t	 posModes_r17;
		GNSS_ID_Bitmap_t	 gnss_TimeIDs_r17;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *gnssTime_r17;
	struct PositioningModes	*e_utraTime_r17;	/* OPTIONAL */
	struct PositioningModes	*nrTime_r17;	/* OPTIONAL */
	struct PositioningModes	*relativeTime_r17;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ScheduledLocationTimeSupportPerMode_r17_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ScheduledLocationTimeSupportPerMode_r17;
extern asn_SEQUENCE_specifics_t asn_SPC_ScheduledLocationTimeSupportPerMode_r17_specs_1;
extern asn_TYPE_member_t asn_MBR_ScheduledLocationTimeSupportPerMode_r17_1[5];

#ifdef __cplusplus
}
#endif

#endif	/* _ScheduledLocationTimeSupportPerMode_r17_H_ */
#include <asn_internal.h>