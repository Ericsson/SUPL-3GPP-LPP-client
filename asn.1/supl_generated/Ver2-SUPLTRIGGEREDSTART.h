/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SUPL-TRIGGERED-START"
 * 	found in "src/SUPL-TRIGGERED-START.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -no-gen-example -pdu=all -gen-XER -gen-JER -no-gen-OER -gen-UPER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -D supl_generated/ -S empty_skeleton/`
 */

#ifndef	_Ver2_SUPLTRIGGEREDSTART_H_
#define	_Ver2_SUPLTRIGGEREDSTART_H_


#include <asn_application.h>

/* Including external dependencies */
#include "SETCapabilities.h"
#include "LocationId.h"
#include "Ver.h"
#include "TriggerType.h"
#include "CauseCode.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct QoP;
struct MultipleLocationIds;
struct ThirdParty;
struct ApplicationID;
struct TriggerParams;
struct Position;
struct ReportingCap;

/* Ver2-SUPLTRIGGEREDSTART */
typedef struct Ver2_SUPLTRIGGEREDSTART {
	SETCapabilities_t	 sETCapabilities;
	LocationId_t	 locationId;
	Ver_t	*ver;	/* OPTIONAL */
	struct QoP	*qoP;	/* OPTIONAL */
	struct MultipleLocationIds	*multipleLocationIds;	/* OPTIONAL */
	struct ThirdParty	*thirdParty;	/* OPTIONAL */
	struct ApplicationID	*applicationID;	/* OPTIONAL */
	TriggerType_t	*triggerType;	/* OPTIONAL */
	struct TriggerParams	*triggerParams;	/* OPTIONAL */
	struct Position	*position;	/* OPTIONAL */
	struct ReportingCap	*reportingCap;	/* OPTIONAL */
	CauseCode_t	*causeCode;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Ver2_SUPLTRIGGEREDSTART_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ver2_SUPLTRIGGEREDSTART;
extern asn_SEQUENCE_specifics_t asn_SPC_Ver2_SUPLTRIGGEREDSTART_specs_1;
extern asn_TYPE_member_t asn_MBR_Ver2_SUPLTRIGGEREDSTART_1[12];

#ifdef __cplusplus
}
#endif

#endif	/* _Ver2_SUPLTRIGGEREDSTART_H_ */
#include <asn_internal.h>