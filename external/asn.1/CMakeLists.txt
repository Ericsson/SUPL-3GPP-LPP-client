
set(ASN1_XER_SUPPORT ON CACHE BOOL "Enable XER support")
set(ASN1_JER_SUPPORT ON CACHE BOOL "Enable JER support")
set(ASN1_OER_SUPPORT OFF CACHE BOOL "Enable OER support")
set(ASN1_UPER_SUPPORT ON CACHE BOOL "Enable APER support")
set(ASN1_APER_SUPPORT OFF CACHE BOOL "Enable APER support")
set(ASN1_BER_SUPPORT OFF CACHE BOOL "Enable BER support")
set(ASN1_RFILL_SUPPORT OFF CACHE BOOL "Enable RFILL support")
set(ASN1_PRINT_SUPPORT OFF CACHE BOOL "Enable PRINT support")

function(asn1_target_setup target)
    if(NOT ASN1_XER_SUPPORT)
        target_compile_definitions(${target} PUBLIC "ASN_DISABLE_XER_SUPPORT=1")
    endif()

    if(NOT ASN1_JER_SUPPORT)
        target_compile_definitions(${target} PUBLIC "ASN_DISABLE_JER_SUPPORT=1")
    endif()

    if(NOT ASN1_OER_SUPPORT)
        target_compile_definitions(${target} PUBLIC "ASN_DISABLE_OER_SUPPORT=1")
    endif()

    if(NOT ASN1_UPER_SUPPORT)
        target_compile_definitions(${target} PUBLIC "ASN_DISABLE_UPER_SUPPORT=1")
    endif()

    if(NOT ASN1_APER_SUPPORT)
        target_compile_definitions(${target} PUBLIC "ASN_DISABLE_APER_SUPPORT=1")
    endif()

    if(NOT ASN1_BER_SUPPORT)
        target_compile_definitions(${target} PUBLIC "ASN_DISABLE_BER_SUPPORT=1")
    endif()

    if(NOT ASN1_RFILL_SUPPORT)
        target_compile_definitions(${target} PUBLIC "ASN_DISABLE_RFILL_SUPPORT=1")
    endif()

    if(NOT ASN1_PRINT_SUPPORT)
        target_compile_definitions(${target} PUBLIC "ASN_DISABLE_PRINT_SUPPORT=1")
    endif()

    # Force optimizations for ASN.1 code even in Debug builds
    # ASN.1 decoding is performance-critical and the generated code is stable
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target} PRIVATE
            "-O3"
            "-Wno-parentheses-equality" 
            "-Wno-unused-but-set-variable"
        )
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE "-O3")
    endif()

    # ASN.1 generated code should not be compiled with ASAN as it significantly
    # impacts performance (6+ second delays observed in encoding/decoding)
endfunction()

#
# Skeleton ASN.1
#

set(ASN1_SKELETON_SOURCE "")

if (ASN1_XER_SUPPORT)
    file(GLOB_RECURSE ASN1_XER_SOURCE "skeleton/*_xer.c")
    list(APPEND ASN1_SKELETON_SOURCE ${ASN1_XER_SOURCE})
    list(APPEND ASN1_SKELETON_SOURCE 
        "skeleton/xer_decoder.c"
        "skeleton/xer_decoder.h"
        "skeleton/xer_encoder.c"
        "skeleton/xer_encoder.h"
        "skeleton/xer_support.c"
        "skeleton/xer_support.h"
    )
endif()

if (ASN1_JER_SUPPORT)
    file(GLOB_RECURSE ASN1_JER_SOURCE "skeleton/*_jer.c")
    list(APPEND ASN1_SKELETON_SOURCE ${ASN1_JER_SOURCE})
    list(APPEND ASN1_SKELETON_SOURCE 
        "skeleton/jer_decoder.c"
        "skeleton/jer_decoder.h"
        "skeleton/jer_encoder.c"
        "skeleton/jer_encoder.h"
        "skeleton/jer_support.c"
        "skeleton/jer_support.h"
    )
endif()

if (ASN1_OER_SUPPORT)
    file(GLOB_RECURSE ASN1_OER_SOURCE "skeleton/*_oer.c")
    list(APPEND ASN1_SKELETON_SOURCE ${ASN1_OER_SOURCE})
    list(APPEND ASN1_SKELETON_SOURCE 
        "skeleton/oer_decoder.c"
        "skeleton/oer_decoder.h"
        "skeleton/oer_encoder.c"
        "skeleton/oer_encoder.h"
        "skeleton/oer_support.c"
        "skeleton/oer_support.h"
    )
endif()

if (ASN1_UPER_SUPPORT)
    file(GLOB_RECURSE ASN1_UPER_SOURCE "skeleton/*_uper.c")
    list(APPEND ASN1_SKELETON_SOURCE ${ASN1_UPER_SOURCE})
    list(APPEND ASN1_SKELETON_SOURCE 
        "skeleton/uper_decoder.c"
        "skeleton/uper_decoder.h"
        "skeleton/uper_encoder.c"
        "skeleton/uper_encoder.h"
        "skeleton/uper_opentype.c"
        "skeleton/uper_opentype.h"
        "skeleton/uper_support.c"
        "skeleton/uper_support.h"
    )
endif()

if (ASN1_APER_SUPPORT)
    file(GLOB_RECURSE ASN1_APER_SOURCE "skeleton/*_aper.c")
    list(APPEND ASN1_SKELETON_SOURCE ${ASN1_APER_SOURCE})
    list(APPEND ASN1_SKELETON_SOURCE
        "skeleton/aper_decoder.c"
        "skeleton/aper_decoder.h"
        "skeleton/aper_encoder.c"
        "skeleton/aper_encoder.h"
        "skeleton/aper_opentype.c"
        "skeleton/aper_opentype.h"
        "skeleton/aper_support.c"
        "skeleton/aper_support.h"
    )
endif()

# UPER OR APER
if (ASN1_UPER_SUPPORT OR ASN1_APER_SUPPORT)
    list(APPEND ASN1_SKELETON_SOURCE 
        "skeleton/per_decoder.c"
        "skeleton/per_decoder.h"
        "skeleton/per_encoder.c"
        "skeleton/per_encoder.h"
        "skeleton/per_opentype.c"
        "skeleton/per_opentype.h"
        "skeleton/per_support.c"
        "skeleton/per_support.h"
    )
endif()

if (ASN1_BER_SUPPORT)
    file(GLOB_RECURSE ASN1_BER_SOURCE "skeleton/*_ber.c")
    list(APPEND ASN1_SKELETON_SOURCE ${ASN1_BER_SOURCE})
    list(APPEND ASN1_SKELETON_SOURCE 
        "skeleton/ber_decoder.c"
        "skeleton/ber_decoder.h"
        "skeleton/ber_tlv_length.c"
        "skeleton/ber_tlv_length.h"
        "skeleton/ber_tlv_tag.c"
        "skeleton/ber_tlv_tag.h"
        "skeleton/der_encoder.c"
        "skeleton/der_encoder.h"
    )
endif()

if (ASN1_RFILL_SUPPORT)
    file(GLOB_RECURSE ASN1_RFILL_SOURCE "skeleton/*_rfill.c")
    list(APPEND ASN1_SKELETON_SOURCE ${ASN1_RFILL_SOURCE})
    list(APPEND ASN1_SKELETON_SOURCE 
        "skeleton/asn_random_fill.c"
        "skeleton/asn_random_fill.h"
    )
endif()

if (ASN1_PRINT_SUPPORT)
    file(GLOB_RECURSE ASN1_PRINT_SOURCE "skeleton/*_print.c")
    list(APPEND ASN1_SKELETON_SOURCE ${ASN1_PRINT_SOURCE})
endif()

# Common skeleton files
list(APPEND ASN1_SKELETON_SOURCE 
    "skeleton/ANY.c"
    "skeleton/ANY.h"
    "skeleton/BIT_STRING.c"
    "skeleton/BIT_STRING.h"
    "skeleton/BMPString.c"
    "skeleton/BMPString.h"
    "skeleton/BOOLEAN.c"
    "skeleton/BOOLEAN.h"
    "skeleton/ENUMERATED.c"
    "skeleton/ENUMERATED.h"
    "skeleton/GeneralString.c"
    "skeleton/GeneralString.h"
    "skeleton/GeneralizedTime.c"
    "skeleton/GeneralizedTime.h"
    "skeleton/GraphicString.c"
    "skeleton/GraphicString.h"
    "skeleton/IA5String.c"
    "skeleton/IA5String.h"
    "skeleton/INTEGER.c"
    "skeleton/INTEGER.h"
    "skeleton/ISO646String.c"
    "skeleton/ISO646String.h"
    "skeleton/NULL.c"
    "skeleton/NULL.h"
    "skeleton/NativeEnumerated.c"
    "skeleton/NativeEnumerated.h"
    "skeleton/NativeInteger.c"
    "skeleton/NativeInteger.h"
    "skeleton/NativeReal.c"
    "skeleton/NativeReal.h"
    "skeleton/NumericString.c"
    "skeleton/NumericString.h"
    "skeleton/OBJECT_IDENTIFIER.c"
    "skeleton/OBJECT_IDENTIFIER.h"
    "skeleton/OCTET_STRING.c"
    "skeleton/OCTET_STRING.h"
    "skeleton/OPEN_TYPE.c"
    "skeleton/OPEN_TYPE.h"
    "skeleton/ObjectDescriptor.c"
    "skeleton/ObjectDescriptor.h"
    "skeleton/PrintableString.c"
    "skeleton/PrintableString.h"
    "skeleton/REAL.c"
    "skeleton/REAL.h"
    "skeleton/RELATIVE-OID.c"
    "skeleton/RELATIVE-OID.h"
    "skeleton/T61String.c"
    "skeleton/T61String.h"
    "skeleton/TeletexString.c"
    "skeleton/TeletexString.h"
    "skeleton/UTCTime.c"
    "skeleton/UTCTime.h"
    "skeleton/UTF8String.c"
    "skeleton/UTF8String.h"
    "skeleton/UniversalString.c"
    "skeleton/UniversalString.h"
    "skeleton/VideotexString.c"
    "skeleton/VideotexString.h"
    "skeleton/VisibleString.c"
    "skeleton/VisibleString.h"
    "skeleton/asn_SEQUENCE_OF.c"
    "skeleton/asn_SEQUENCE_OF.h"
    "skeleton/asn_SET_OF.c"
    "skeleton/asn_SET_OF.h"
    "skeleton/asn_application.c"
    "skeleton/asn_application.h"
    "skeleton/asn_bit_data.c"
    "skeleton/asn_bit_data.h"
    "skeleton/asn_codecs.h"
    "skeleton/asn_codecs_prim.c"
    "skeleton/asn_codecs_prim.h"
    "skeleton/asn_config.h"
    "skeleton/asn_internal.c"
    "skeleton/asn_internal.h"
    "skeleton/asn_ioc.h"
    "skeleton/asn_system.h"
    "skeleton/constr_CHOICE.c"
    "skeleton/constr_CHOICE.h"
    "skeleton/constr_SEQUENCE.c"
    "skeleton/constr_SEQUENCE.h"
    "skeleton/constr_SEQUENCE_OF.c"
    "skeleton/constr_SEQUENCE_OF.h"
    "skeleton/constr_SET.c"
    "skeleton/constr_SET.h"
    "skeleton/constr_SET_OF.c"
    "skeleton/constr_SET_OF.h"
    "skeleton/constr_TYPE.c"
    "skeleton/constr_TYPE.h"
    "skeleton/constraints.c"
    "skeleton/constraints.h"
)

add_library(asn1_skeleton STATIC ${ASN1_SKELETON_SOURCE})
add_library(asn1::skeleton ALIAS asn1_skeleton)

asn1_target_setup(asn1_skeleton)
target_include_directories(asn1_skeleton PUBLIC "skeleton")
set_property(TARGET asn1_skeleton PROPERTY C_STANDARD 99)
set_target_properties(asn1_skeleton PROPERTIES
    C_CLANG_TIDY ""
    C_CPPCHECK ""
    C_INCLUDE_WHAT_YOU_USE ""
    UNITY_BUILD OFF
)
if(COMPILER_SUPPORTS_NO_SANITIZE)
    # Explicitly disable ASAN for ASN.1 code
    target_compile_options(asn1_skeleton PRIVATE -fno-sanitize=all)
    target_link_options(asn1_skeleton PRIVATE -fno-sanitize=all)
endif()

#
# SUPL ASN.1
#

file(GLOB_RECURSE ASN1_SUPL_SOURCE "supl_generated/*.c")
file(GLOB_RECURSE ASN1_SUPL_HEADER "supl_generated/*.h")

add_library(asn1_supl_generated STATIC ${ASN1_SUPL_SOURCE} ${ASN1_SUPL_HEADER})
add_library(asn1::generated::supl ALIAS asn1_supl_generated)

asn1_target_setup(asn1_supl_generated)
target_link_libraries(asn1_supl_generated PUBLIC asn1::skeleton)
target_include_directories(asn1_supl_generated PUBLIC "supl_generated")
set_target_properties(asn1_supl_generated PROPERTIES
    C_CLANG_TIDY ""
    C_CPPCHECK ""
    C_INCLUDE_WHAT_YOU_USE ""
    UNITY_BUILD OFF
)
if(COMPILER_SUPPORTS_NO_SANITIZE)
    # Explicitly disable ASAN for ASN.1 code
    target_compile_options(asn1_supl_generated PRIVATE -fno-sanitize=all)
    target_link_options(asn1_supl_generated PRIVATE -fno-sanitize=all)
endif()

#
# 3GPP LPP ASN.1
#

file(GLOB_RECURSE ASN1_LPP_SOURCE "lpp_generated/*.c")
file(GLOB_RECURSE ASN1_LPP_HEADER "lpp_generated/*.h")

add_library(asn1_lpp_generated STATIC ${ASN1_LPP_SOURCE} ${ASN1_LPP_HEADER})
add_library(asn1::generated::lpp ALIAS asn1_lpp_generated)

asn1_target_setup(asn1_lpp_generated)
target_link_libraries(asn1_lpp_generated PUBLIC asn1::skeleton)
target_include_directories(asn1_lpp_generated PUBLIC "lpp_generated")
set_target_properties(asn1_lpp_generated PROPERTIES
    C_CLANG_TIDY ""
    C_CPPCHECK ""
    C_INCLUDE_WHAT_YOU_USE ""
    UNITY_BUILD OFF
)
if(COMPILER_SUPPORTS_NO_SANITIZE)
    # Explicitly disable ASAN for ASN.1 code
    target_compile_options(asn1_lpp_generated PRIVATE -fno-sanitize=all)
    target_link_options(asn1_lpp_generated PRIVATE -fno-sanitize=all)
endif()

#
# Helper
#

add_subdirectory(helper)
