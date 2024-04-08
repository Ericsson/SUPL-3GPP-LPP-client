/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ULP-Version-2-parameter-extensions"
 * 	found in "src/SUPL.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_EventTriggerCapabilities_H_
#define	_EventTriggerCapabilities_H_


#include <asn_application.h>

/* Including external dependencies */
#include "GeoAreaShapesSupported.h"
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* EventTriggerCapabilities */
typedef struct EventTriggerCapabilities {
	GeoAreaShapesSupported_t	 geoAreaShapesSupported;
	long	*maxNumGeoAreaSupported;	/* OPTIONAL */
	long	*maxAreaIdListSupported;	/* OPTIONAL */
	long	*maxAreaIdSupportedPerList;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} EventTriggerCapabilities_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EventTriggerCapabilities;
extern asn_SEQUENCE_specifics_t asn_SPC_EventTriggerCapabilities_specs_1;
extern asn_TYPE_member_t asn_MBR_EventTriggerCapabilities_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _EventTriggerCapabilities_H_ */
#include <asn_internal.h>
