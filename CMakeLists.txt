cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER_WORKS)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(MbedutilsDevelopment C CXX)
enable_testing()

add_subdirectory(lib/etl)
add_subdirectory(lib/mbedutils_freertos)
add_subdirectory(lib/mbedutils_pico)
add_subdirectory(lib/mbedutils_sim)
add_subdirectory(mbedutils)
add_subdirectory(tests)

# Add a custom target for generating coverage reports
find_program(LCOV_PATH lcov)
find_program(GCOVR_PATH gcovr)
find_program(GENHTML_PATH genhtml)

set(COVERAGE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/coverage)
add_custom_target(UnitTests
  COMMAND ${CMAKE_MAKE_PROGRAM} UnitTestMbedutils
  COMMAND mkdir -p "${COVERAGE_OUTPUT_DIRECTORY}"
  COMMAND ${CMAKE_COMMAND} -E chdir ${PROJECT_BINARY_DIR} ctest --output-on-failure
  COMMAND ${GCOVR_PATH} -r ${CMAKE_CURRENT_SOURCE_DIR}/mbedutils/src --object-directory=${CMAKE_CURRENT_BINARY_DIR} --html-details -o ${COVERAGE_OUTPUT_DIRECTORY}/coverage.html
  COMMAND echo "Coverage report generated at ${COVERAGE_OUTPUT_DIRECTORY}/coverage.html"
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)
