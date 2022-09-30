
file(GLOB_RECURSE SRC_CPP "*.cpp")
file(GLOB_RECURSE SRC_HPP "../include/*.h" "../include/*.hpp")

add_executable(example ${SRC_CPP} ${SRC_HPP})
target_include_directories(example PRIVATE ../include/)
target_link_libraries(example PRIVATE utility modem rtklib asnlib lpplib osr rtcm_generator Threads::Threads)

if (USE_OPENSSL)
target_link_libraries(example PRIVATE OpenSSL::SSL)
target_compile_definitions(example PRIVATE "USE_OPENSSL=1")
endif (USE_OPENSSL)

if (USE_ASAN)
target_compile_options(utility PRIVATE -fsanitize=address)
target_link_libraries(utility PRIVATE -fsanitize=address)
endif (USE_ASAN)
