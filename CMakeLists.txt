cmake_minimum_required(VERSION 3.10)
project(OtusTests)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(tests main.cpp ExceptionHandler.cpp RegisterCommand.cpp
ClearScopeCommand.cpp CreateScopeCommand.cpp SetCurrentScopeCommand.cpp IoC.cpp)

target_link_libraries(tests ${GTEST_LIBRARIES})

add_test(NAME tests COMMAND tests)
