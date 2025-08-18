cmake_minimum_required(VERSION 3.10)
project(OtusUnitTests)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(unit_tests main.cpp)

target_link_libraries(unit_tests ${GTEST_LIBRARIES})

add_test(NAME unit_tests COMMAND unit_tests)
