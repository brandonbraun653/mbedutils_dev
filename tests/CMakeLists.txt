project(mbedutils_test)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG)
endif()

# Import CppUTest
add_subdirectory(${PROJECT_SOURCE_DIR}/../lib/cpputest ${CMAKE_CURRENT_BINARY_DIR}/cpputest)

# Include Dependent Headers
include_directories(
    ${PROJECT_SOURCE_DIR}/../mbedutils/include
    ${PROJECT_SOURCE_DIR}/../mbedutils/lib/cobs
    ${PROJECT_SOURCE_DIR}/../mbedutils/lib/nanopb
    ${PROJECT_SOURCE_DIR}/../mbedutils/src/rpc/proto
    ${PROJECT_SOURCE_DIR}/../lib/cpputest/include
    ${PROJECT_SOURCE_DIR}/../lib/etl/include
)

# Get the entire set of mbedutils sources
file(GLOB_RECURSE MBEDUTILS_SOURCES ${PROJECT_SOURCE_DIR}/../mbedutils/src/*.cpp)
file(GLOB_RECURSE MBEDUTILS_MOCKS ${PROJECT_SOURCE_DIR}/mocks/*.cpp)
file(GLOB_RECURSE MBEDUTILS_TESTS ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Create a library with the mbedutils sources, mocks, and tests. Enables options to help
# with generating code coverage reports.
add_library(MbedutilsDrivers STATIC ${MBEDUTILS_SOURCES})
target_compile_options(MbedutilsDrivers PRIVATE -fprofile-arcs -ftest-coverage --coverage -g -O0)
target_link_options(MbedutilsDrivers PRIVATE -fprofile-arcs -ftest-coverage)
target_link_libraries(MbedutilsDrivers PUBLIC
    gcov
    etl::etl
    mbedutils_headers
    mbedutils_internal_headers
)

# Create the test executable
add_executable(UnitTestMbedutils run_all_tests.cpp ${MBEDUTILS_TESTS} ${MBEDUTILS_MOCKS})
target_link_libraries(UnitTestMbedutils
    CppUTest
    CppUTestExt
    gcov
    MbedutilsDrivers
    mbedutils_lib_stl
)

# Fix annoying linker ordering dependencies that might get introduced
target_link_options(UnitTestMbedutils PRIVATE -Wl,--start-group)

# Add the test to the CMake test suite
add_test(NAME UnitTestMbedutils COMMAND UnitTestMbedutils)
