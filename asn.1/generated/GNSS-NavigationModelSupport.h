/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_GNSS_NavigationModelSupport_H_
#define	_GNSS_NavigationModelSupport_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum GNSS_NavigationModelSupport__clockModel {
	GNSS_NavigationModelSupport__clockModel_model_1	= 0,
	GNSS_NavigationModelSupport__clockModel_model_2	= 1,
	GNSS_NavigationModelSupport__clockModel_model_3	= 2,
	GNSS_NavigationModelSupport__clockModel_model_4	= 3,
	GNSS_NavigationModelSupport__clockModel_model_5	= 4,
	GNSS_NavigationModelSupport__clockModel_model_6	= 5,
	GNSS_NavigationModelSupport__clockModel_model_7_r16	= 6,
	GNSS_NavigationModelSupport__clockModel_model_8_r16	= 7
} e_GNSS_NavigationModelSupport__clockModel;
typedef enum GNSS_NavigationModelSupport__orbitModel {
	GNSS_NavigationModelSupport__orbitModel_model_1	= 0,
	GNSS_NavigationModelSupport__orbitModel_model_2	= 1,
	GNSS_NavigationModelSupport__orbitModel_model_3	= 2,
	GNSS_NavigationModelSupport__orbitModel_model_4	= 3,
	GNSS_NavigationModelSupport__orbitModel_model_5	= 4,
	GNSS_NavigationModelSupport__orbitModel_model_6	= 5,
	GNSS_NavigationModelSupport__orbitModel_model_7_r16	= 6,
	GNSS_NavigationModelSupport__orbitModel_model_8_r16	= 7
} e_GNSS_NavigationModelSupport__orbitModel;

/* GNSS-NavigationModelSupport */
typedef struct GNSS_NavigationModelSupport {
	BIT_STRING_t	*clockModel;	/* OPTIONAL */
	BIT_STRING_t	*orbitModel;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GNSS_NavigationModelSupport_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GNSS_NavigationModelSupport;
extern asn_SEQUENCE_specifics_t asn_SPC_GNSS_NavigationModelSupport_specs_1;
extern asn_TYPE_member_t asn_MBR_GNSS_NavigationModelSupport_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _GNSS_NavigationModelSupport_H_ */
#include <asn_internal.h>
