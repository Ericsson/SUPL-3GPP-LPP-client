/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "/home/martin/repos/LPP-Client/asn/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_GNSS_ReferenceLocation_H_
#define	_GNSS_ReferenceLocation_H_


#include <asn_application.h>

/* Including external dependencies */
#include "EllipsoidPointWithAltitudeAndUncertaintyEllipsoid.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GNSS-ReferenceLocation */
typedef struct GNSS_ReferenceLocation {
	EllipsoidPointWithAltitudeAndUncertaintyEllipsoid_t	 threeDlocation;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GNSS_ReferenceLocation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GNSS_ReferenceLocation;
extern asn_SEQUENCE_specifics_t asn_SPC_GNSS_ReferenceLocation_specs_1;
extern asn_TYPE_member_t asn_MBR_GNSS_ReferenceLocation_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _GNSS_ReferenceLocation_H_ */
#include <asn_internal.h>
