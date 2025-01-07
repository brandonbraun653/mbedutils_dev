cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER_WORKS)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(MbedutilsDevelopment C CXX)

# Add a custom target for generating coverage reports
find_program(LCOV_PATH lcov)
find_program(GCOVR_PATH gcovr)
find_program(GENHTML_PATH genhtml)

set(ARCHIVE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/archive)
set(COVERAGE_OUTPUT_DIR ${ARCHIVE_OUTPUT_DIR}/coverage)
set(MBEDUTILS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mbedutils/src)
set(MBEDUTILS_SRC_DIR_SIM ${CMAKE_CURRENT_SOURCE_DIR}/lib/mbedutils_sim)
set(MBEDUTILS_SRC_DIR_FREERTOS ${CMAKE_CURRENT_SOURCE_DIR}/lib/mbedutils_freertos)
add_custom_target(UnitTests
  COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "Building all tests..."
  COMMAND ${CMAKE_COMMAND} -E env ${CMAKE_MAKE_PROGRAM} BuildAllTests
  COMMAND mkdir -p "${ARCHIVE_OUTPUT_DIR}"
  COMMAND mkdir -p "${COVERAGE_OUTPUT_DIR}"
  COMMAND ${CMAKE_COMMAND} -E chdir ${PROJECT_BINARY_DIR} ctest --output-on-failure --no-compress-output --output-junit ${ARCHIVE_OUTPUT_DIR}/junit_results.xml
  COMMAND ${GCOVR_PATH} --root ${CMAKE_CURRENT_SOURCE_DIR} --filter ${MBEDUTILS_SRC_DIR} --filter ${MBEDUTILS_SRC_DIR_SIM} --filter ${MBEDUTILS_SRC_DIR_FREERTOS} --object-directory=${CMAKE_CURRENT_BINARY_DIR} --html-details -o ${COVERAGE_OUTPUT_DIR}/index.html
  COMMAND echo "Coverage report generated at ${COVERAGE_OUTPUT_DIR}/index.html"
  COMMAND echo "JUnit test results generated at ${ARCHIVE_OUTPUT_DIR}/junit_results.xml"
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)

# Add testing dependencies
add_subdirectory(lib/etl)
add_subdirectory(lib/mbedutils_freertos)
add_subdirectory(lib/mbedutils_pico)
add_subdirectory(lib/mbedutils_sim)
add_subdirectory(lib/mbedutils_test)
add_subdirectory(mbedutils)

# Add tests
enable_testing()
add_subdirectory(tests)