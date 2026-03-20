add_executable(example_ppp "main.cpp")
add_executable(example::ppp ALIAS example_ppp)

target_link_libraries(example_ppp PRIVATE dependency::generator::idokeido)
target_link_libraries(example_ppp PRIVATE dependency::format::rinex)
target_link_libraries(example_ppp PRIVATE dependency::ephemeris)
target_link_libraries(example_ppp PRIVATE dependency::loglet)

setup_target(example_ppp)

add_executable(convert_sweref99 "convert_sweref99.cpp")
target_link_libraries(convert_sweref99 PRIVATE dependency::coordinates)
setup_target(convert_sweref99)
