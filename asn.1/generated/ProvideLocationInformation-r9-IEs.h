/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LPP-PDU-Definitions"
 * 	found in "src/LPP.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_ProvideLocationInformation_r9_IEs_H_
#define	_ProvideLocationInformation_r9_IEs_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct CommonIEsProvideLocationInformation;
struct A_GNSS_ProvideLocationInformation;
struct OTDOA_ProvideLocationInformation;
struct ECID_ProvideLocationInformation;
struct EPDU_Sequence;
struct Sensor_ProvideLocationInformation_r13;
struct TBS_ProvideLocationInformation_r13;
struct WLAN_ProvideLocationInformation_r13;
struct BT_ProvideLocationInformation_r13;
struct NR_ECID_ProvideLocationInformation_r16;
struct NR_Multi_RTT_ProvideLocationInformation_r16;
struct NR_DL_AoD_ProvideLocationInformation_r16;
struct NR_DL_TDOA_ProvideLocationInformation_r16;

/* ProvideLocationInformation-r9-IEs */
typedef struct ProvideLocationInformation_r9_IEs {
	struct CommonIEsProvideLocationInformation	*commonIEsProvideLocationInformation;	/* OPTIONAL */
	struct A_GNSS_ProvideLocationInformation	*a_gnss_ProvideLocationInformation;	/* OPTIONAL */
	struct OTDOA_ProvideLocationInformation	*otdoa_ProvideLocationInformation;	/* OPTIONAL */
	struct ECID_ProvideLocationInformation	*ecid_ProvideLocationInformation;	/* OPTIONAL */
	struct EPDU_Sequence	*epdu_ProvideLocationInformation;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct ProvideLocationInformation_r9_IEs__ext1 {
		struct Sensor_ProvideLocationInformation_r13	*sensor_ProvideLocationInformation_r13;	/* OPTIONAL */
		struct TBS_ProvideLocationInformation_r13	*tbs_ProvideLocationInformation_r13;	/* OPTIONAL */
		struct WLAN_ProvideLocationInformation_r13	*wlan_ProvideLocationInformation_r13;	/* OPTIONAL */
		struct BT_ProvideLocationInformation_r13	*bt_ProvideLocationInformation_r13;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext1;
	struct ProvideLocationInformation_r9_IEs__ext2 {
		struct NR_ECID_ProvideLocationInformation_r16	*nr_ECID_ProvideLocationInformation_r16;	/* OPTIONAL */
		struct NR_Multi_RTT_ProvideLocationInformation_r16	*nr_Multi_RTT_ProvideLocationInformation_r16;	/* OPTIONAL */
		struct NR_DL_AoD_ProvideLocationInformation_r16	*nr_DL_AoD_ProvideLocationInformation_r16;	/* OPTIONAL */
		struct NR_DL_TDOA_ProvideLocationInformation_r16	*nr_DL_TDOA_ProvideLocationInformation_r16;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *ext2;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ProvideLocationInformation_r9_IEs_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ProvideLocationInformation_r9_IEs;
extern asn_SEQUENCE_specifics_t asn_SPC_ProvideLocationInformation_r9_IEs_specs_1;
extern asn_TYPE_member_t asn_MBR_ProvideLocationInformation_r9_IEs_1[7];

#ifdef __cplusplus
}
#endif

#endif	/* _ProvideLocationInformation_r9_IEs_H_ */
#include <asn_internal.h>
