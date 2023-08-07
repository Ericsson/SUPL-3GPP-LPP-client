/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "/home/martin/repos/LPP-Client/asn/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_TBS_LocationServerErrorCauses_r13_H_
#define	_TBS_LocationServerErrorCauses_r13_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum TBS_LocationServerErrorCauses_r13__cause_r13 {
	TBS_LocationServerErrorCauses_r13__cause_r13_undefined	= 0,
	/*
	 * Enumeration is extensible
	 */
	TBS_LocationServerErrorCauses_r13__cause_r13_assistanceDataNotSupportedByServer_v1420	= 1,
	TBS_LocationServerErrorCauses_r13__cause_r13_assistanceDataSupportedButCurrentlyNotAvailableByServer_v1420	= 2
} e_TBS_LocationServerErrorCauses_r13__cause_r13;

/* TBS-LocationServerErrorCauses-r13 */
typedef struct TBS_LocationServerErrorCauses_r13 {
	long	 cause_r13;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TBS_LocationServerErrorCauses_r13_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_cause_r13_2;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_TBS_LocationServerErrorCauses_r13;
extern asn_SEQUENCE_specifics_t asn_SPC_TBS_LocationServerErrorCauses_r13_specs_1;
extern asn_TYPE_member_t asn_MBR_TBS_LocationServerErrorCauses_r13_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _TBS_LocationServerErrorCauses_r13_H_ */
#include <asn_internal.h>
