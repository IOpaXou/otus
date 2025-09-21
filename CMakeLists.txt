cmake_minimum_required(VERSION 3.10)
project(OtusTests)

file(GLOB_RECURSE SOURCES "src/*cpp")

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(include include/commands)

add_executable(tests ${SOURCES})

target_link_libraries(tests ${GTEST_LIBRARIES})

add_test(NAME tests COMMAND tests)
