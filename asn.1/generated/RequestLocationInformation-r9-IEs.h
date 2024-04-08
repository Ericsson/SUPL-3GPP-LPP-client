/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_RequestLocationInformation_r9_IEs_H_
#define	_RequestLocationInformation_r9_IEs_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct CommonIEsRequestLocationInformation;
struct A_GNSS_RequestLocationInformation;
struct OTDOA_RequestLocationInformation;
struct ECID_RequestLocationInformation;
struct EPDU_Sequence;
struct Sensor_RequestLocationInformation_r13;
struct TBS_RequestLocationInformation_r13;
struct WLAN_RequestLocationInformation_r13;
struct BT_RequestLocationInformation_r13;
struct NR_ECID_RequestLocationInformation_r16;
struct NR_Multi_RTT_RequestLocationInformation_r16;
struct NR_DL_AoD_RequestLocationInformation_r16;
struct NR_DL_TDOA_RequestLocationInformation_r16;

/* RequestLocationInformation-r9-IEs */
typedef struct RequestLocationInformation_r9_IEs {
	struct CommonIEsRequestLocationInformation	*commonIEsRequestLocationInformation;	/* OPTIONAL */
	struct A_GNSS_RequestLocationInformation	*a_gnss_RequestLocationInformation;	/* OPTIONAL */
	struct OTDOA_RequestLocationInformation	*otdoa_RequestLocationInformation;	/* OPTIONAL */
	struct ECID_RequestLocationInformation	*ecid_RequestLocationInformation;	/* OPTIONAL */
	struct EPDU_Sequence	*epdu_RequestLocationInformation;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct RequestLocationInformation_r9_IEs__ext1 {
		struct Sensor_RequestLocationInformation_r13	*sensor_RequestLocationInformation_r13;	/* OPTIONAL */
		struct TBS_RequestLocationInformation_r13	*tbs_RequestLocationInformation_r13;	/* OPTIONAL */
		struct WLAN_RequestLocationInformation_r13	*wlan_RequestLocationInformation_r13;	/* OPTIONAL */
		struct BT_RequestLocationInformation_r13	*bt_RequestLocationInformation_r13;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext1;
	struct RequestLocationInformation_r9_IEs__ext2 {
		struct NR_ECID_RequestLocationInformation_r16	*nr_ECID_RequestLocationInformation_r16;	/* OPTIONAL */
		struct NR_Multi_RTT_RequestLocationInformation_r16	*nr_Multi_RTT_RequestLocationInformation_r16;	/* OPTIONAL */
		struct NR_DL_AoD_RequestLocationInformation_r16	*nr_DL_AoD_RequestLocationInformation_r16;	/* OPTIONAL */
		struct NR_DL_TDOA_RequestLocationInformation_r16	*nr_DL_TDOA_RequestLocationInformation_r16;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext2;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RequestLocationInformation_r9_IEs_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RequestLocationInformation_r9_IEs;
extern asn_SEQUENCE_specifics_t asn_SPC_RequestLocationInformation_r9_IEs_specs_1;
extern asn_TYPE_member_t asn_MBR_RequestLocationInformation_r9_IEs_1[7];

#ifdef __cplusplus
}
#endif

#endif	/* _RequestLocationInformation_r9_IEs_H_ */
#include <asn_internal.h>
