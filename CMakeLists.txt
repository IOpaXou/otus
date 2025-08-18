cmake_minimum_required(VERSION 3.10)
project(OtusExceptionsTests)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(exceptions_tests main.cpp ExceptionHandler.cpp)

target_link_libraries(exceptions_tests ${GTEST_LIBRARIES})

add_test(NAME exceptions_tests COMMAND exceptions_tests)
