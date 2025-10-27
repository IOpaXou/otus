cmake_minimum_required(VERSION 3.10)
project(OtusTests)

file(GLOB_RECURSE SOURCES "src/*cpp")

find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(include include/commands)

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE)
    message(STATUS "Enabling code coverage")
    add_compile_options(-fprofile-arcs -fprofile-update=atomic -ftest-coverage)
    add_link_options(-fprofile-arcs -fprofile-update=atomic -ftest-coverage)
endif()

add_executable(tests ${SOURCES})

target_link_libraries(tests ${GTEST_LIBRARIES})
target_link_libraries(tests OpenSSL::SSL)

add_test(NAME tests COMMAND tests)

add_custom_target(coverage
    COMMAND ./tests
    COMMAND lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch,gcov,negative
    COMMAND lcov --remove coverage.info '/usr/*'
    '*/include/httplib.h' '*/include/json.hpp' '*/include/json_fwd.hpp'
    --output-file coverage.filtered.info --ignore-errors unused
    COMMAND genhtml coverage.filtered.info --output-directory coverage_report
    COMMAND echo "Coverage report: coverage_report/index.html"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS tests
)
