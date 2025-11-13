set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

file(GLOB CROSS_TUPLE_PATH "/home/builder/x-tools/*")
get_filename_component(CROSS_TUPLE ${CROSS_TUPLE_PATH} NAME)

set(CMAKE_C_COMPILER /home/builder/x-tools/${CROSS_TUPLE}/bin/${CROSS_TUPLE}-gcc)
set(CMAKE_CXX_COMPILER /home/builder/x-tools/${CROSS_TUPLE}/bin/${CROSS_TUPLE}-g++)

# Set the target triple for platform-specific optimizations
set(CMAKE_C_COMPILER_TARGET ${CROSS_TUPLE})
set(CMAKE_CXX_COMPILER_TARGET ${CROSS_TUPLE})

# Set sysroot for headers and libraries
set(CMAKE_SYSROOT /home/builder/x-tools/${CROSS_TUPLE}/${CROSS_TUPLE}/sysroot)

# Additional tools
set(CMAKE_AR /home/builder/x-tools/${CROSS_TUPLE}/bin/${CROSS_TUPLE}-ar)
set(CMAKE_RANLIB /home/builder/x-tools/${CROSS_TUPLE}/bin/${CROSS_TUPLE}-ranlib)
set(CMAKE_STRIP /home/builder/x-tools/${CROSS_TUPLE}/bin/${CROSS_TUPLE}-strip)

set(CMAKE_FIND_ROOT_PATH /home/builder/x-tools/${CROSS_TUPLE})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
