
# add_subdirectory(asnlib)
add_subdirectory(lpplib)
add_subdirectory(modem)
add_subdirectory(args)
add_subdirectory(utility)

if(${EXPERIMENTAL})
add_subdirectory(scheduler)
add_subdirectory(supl)
add_subdirectory(loglet)
add_subdirectory(lpp)
endif()
