project(mbedutils_test)

# Import CppUTest
add_subdirectory(${PROJECT_SOURCE_DIR}/../lib/cpputest ${CMAKE_CURRENT_BINARY_DIR}/cpputest)

# include project source
include_directories(${PROJECT_SOURCE_DIR}/../mbedutils/include)

# include CppUTest headers
include_directories(${PROJECT_SOURCE_DIR}/../lib/cpputest/include)

# Get the entire set of mbedutils sources
file(GLOB_RECURSE MBEDUTILS_SOURCES ${PROJECT_SOURCE_DIR}/../mbedutils/src/*.cpp)
file(GLOB_RECURSE MBEDUTILS_MOCKS ${PROJECT_SOURCE_DIR}/mocks/*.cpp)
file(GLOB_RECURSE MBEDUTILS_TESTS ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Create a library with the mbedutils sources, mocks, and tests. Enables options to help
# with generating code coverage reports.
add_library(MbedutilsDrivers STATIC ${MBEDUTILS_SOURCES} ${MBEDUTILS_MOCKS} ${MBEDUTILS_TESTS})
target_compile_options(MbedutilsDrivers PRIVATE -fprofile-arcs -ftest-coverage --coverage -g -O0)
target_link_options(MbedutilsDrivers PRIVATE -fprofile-arcs -ftest-coverage)
target_link_libraries(MbedutilsDrivers PRIVATE gcov)

# Create the test executable
add_executable(MbedutilsTests run_all_tests.cpp)
target_link_libraries(MbedutilsTests CppUTest CppUTestExt gcov MbedutilsDrivers)

# Add the test to the CMake test suite
add_test(NAME MbedutilsTests COMMAND MbedutilsTests)
