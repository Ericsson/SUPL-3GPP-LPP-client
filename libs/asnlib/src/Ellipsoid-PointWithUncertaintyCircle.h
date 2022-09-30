/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "/home/martin/repos/LPP-Client/asn/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_Ellipsoid_PointWithUncertaintyCircle_H_
#define	_Ellipsoid_PointWithUncertaintyCircle_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Ellipsoid_PointWithUncertaintyCircle__latitudeSign {
	Ellipsoid_PointWithUncertaintyCircle__latitudeSign_north	= 0,
	Ellipsoid_PointWithUncertaintyCircle__latitudeSign_south	= 1
} e_Ellipsoid_PointWithUncertaintyCircle__latitudeSign;

/* Ellipsoid-PointWithUncertaintyCircle */
typedef struct Ellipsoid_PointWithUncertaintyCircle {
	long	 latitudeSign;
	long	 degreesLatitude;
	long	 degreesLongitude;
	long	 uncertainty;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Ellipsoid_PointWithUncertaintyCircle_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_latitudeSign_2;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_Ellipsoid_PointWithUncertaintyCircle;
extern asn_SEQUENCE_specifics_t asn_SPC_Ellipsoid_PointWithUncertaintyCircle_specs_1;
extern asn_TYPE_member_t asn_MBR_Ellipsoid_PointWithUncertaintyCircle_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _Ellipsoid_PointWithUncertaintyCircle_H_ */
#include <asn_internal.h>
