/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SUPL-INIT"
 * 	found in "src/SUPL.asn"
 * 	`asn1c -fcompound-names -no-gen-OER -no-gen-APER -no-gen-BER -no-gen-print -no-gen-random-fill -no-gen-example -fno-include-deps -pdu=all -S /usr/local/share/asn1c -D generated/ -S empty_skeleton/`
 */

#ifndef	_NotificationType_H_
#define	_NotificationType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum NotificationType {
	NotificationType_noNotificationNoVerification	= 0,
	NotificationType_notificationOnly	= 1,
	NotificationType_notificationAndVerficationAllowedNA	= 2,
	NotificationType_notificationAndVerficationDeniedNA	= 3,
	NotificationType_privacyOverride	= 4
	/*
	 * Enumeration is extensible
	 */
} e_NotificationType;

/* NotificationType */
typedef long	 NotificationType_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_NotificationType_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_NotificationType;
extern const asn_INTEGER_specifics_t asn_SPC_NotificationType_specs_1;
asn_struct_free_f NotificationType_free;
asn_constr_check_f NotificationType_constraint;
xer_type_decoder_f NotificationType_decode_xer;
xer_type_encoder_f NotificationType_encode_xer;
jer_type_encoder_f NotificationType_encode_jer;
per_type_decoder_f NotificationType_decode_uper;
per_type_encoder_f NotificationType_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _NotificationType_H_ */
#include <asn_internal.h>
