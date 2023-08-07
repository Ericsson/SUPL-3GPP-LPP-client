/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ULP-Version-2-message-extensions"
 * 	found in "/home/martin/repos/LPP-Client/asn/SUPL.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -pdu=all -no-gen-example -S /home/martin/repos/LPP-Client/ASN1C/skeletons`
 */

#ifndef	_BasicProtectionParams_H_
#define	_BasicProtectionParams_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>
#include <NativeInteger.h>
#include <BIT_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* BasicProtectionParams */
typedef struct BasicProtectionParams {
	OCTET_STRING_t	 keyIdentifier;
	long	 basicReplayCounter;
	BIT_STRING_t	 basicMAC;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} BasicProtectionParams_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_BasicProtectionParams;
extern asn_SEQUENCE_specifics_t asn_SPC_BasicProtectionParams_specs_1;
extern asn_TYPE_member_t asn_MBR_BasicProtectionParams_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _BasicProtectionParams_H_ */
#include <asn_internal.h>
