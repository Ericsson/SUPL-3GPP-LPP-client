
add_library (rtklib STATIC 
    "src/convgpx.c"     "src/datum.c"           "src/geoid.c"           "src/lambda.c"   
    "src/postpos.c"     "src/preceph.c"         "src/rinex.c"           "src/rtcm3.c"   
    "src/rtklib.h"      "src/sbas.c"            "src/stream.c"          "src/tle.c"
    "src/convkml.c"     "src/download.c"        "src/gis.c"             "src/options.c"  
    "src/ppp.c"         "src/rtcm.c"            "src/rtcm3e.c"          "src/rtkpos.c"  
    "src/solution.c"    "src/streamsvr.c"       "src/convrnx.c"         "src/ephemeris.c"  
    "src/ionex.c"       "src/pntpos.c"          "src/ppp_ar.c"          "src/rcvraw.c"   
    "src/rtcm2.c"       "src/rtkcmn.c"          "src/rtksvr.c"          "src/tides.c" 
    "src/rcv/ublox.c"   "src/rcv/swiftnav.c"    "src/rcv/crescent.c"    "src/rcv/skytraq.c"
    "src/rcv/javad.c"   "src/rcv/nvs.c"         "src/rcv/binex.c"       "src/rcv/rt17.c"
    "src/rcv/septentrio.c" "src/rcv/novatel.c")

target_compile_definitions(rtklib PUBLIC 
    "ASN_DISABLE_OER_SUPPORT=1"
    "ENAGLO" "ENAGAL" "ENACMP" 
    "NFREQ=4" "NEXOBS=8")
target_compile_options(rtklib PRIVATE "-std=c99" "-Wno-unused-but-set-variable" "-Wno-unused-variable")
target_include_directories(rtklib PUBLIC "src")

if (USE_ASAN)
target_compile_options(rtklib PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(rtklib PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)