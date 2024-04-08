/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_DL_PRS_Resource_ARP_Element_r16_H_
#define	_DL_PRS_Resource_ARP_Element_r16_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct RelativeLocation_r16;

/* DL-PRS-Resource-ARP-Element-r16 */
typedef struct DL_PRS_Resource_ARP_Element_r16 {
	struct RelativeLocation_r16	*dl_PRS_Resource_ARP_location_r16;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DL_PRS_Resource_ARP_Element_r16_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DL_PRS_Resource_ARP_Element_r16;
extern asn_SEQUENCE_specifics_t asn_SPC_DL_PRS_Resource_ARP_Element_r16_specs_1;
extern asn_TYPE_member_t asn_MBR_DL_PRS_Resource_ARP_Element_r16_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _DL_PRS_Resource_ARP_Element_r16_H_ */
#include <asn_internal.h>
