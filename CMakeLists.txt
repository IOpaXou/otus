cmake_minimum_required(VERSION 3.10)
project(OtusStableAbstractsTests)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(stable_abstracts_tests main.cpp)

target_link_libraries(stable_abstracts_tests ${GTEST_LIBRARIES})

add_test(NAME stable_abstracts_tests COMMAND stable_abstracts_tests)
