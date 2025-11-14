
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   add_definitions(-DCOMPILER_CANNOT_DEDUCE_UNREACHABLE=1)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   add_definitions(-DCOMPILER_CANNOT_DEDUCE_UNREACHABLE=0)
endif()

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fno-sanitize=all" COMPILER_SUPPORTS_NO_SANITIZE)

