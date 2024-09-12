include(test_target.cmake)

project(mbedutils_test)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG)
endif()

# Import CppUTest
add_subdirectory(${PROJECT_SOURCE_DIR}/../lib/cpputest ${CMAKE_CURRENT_BINARY_DIR}/cpputest)

# Include Dependent Headers
set(INCLUDE_DIRECTORIES
    ${PROJECT_SOURCE_DIR}/../mbedutils/include
    ${PROJECT_SOURCE_DIR}/../mbedutils/lib/cobs
    ${PROJECT_SOURCE_DIR}/../mbedutils/lib/nanopb
    ${PROJECT_SOURCE_DIR}/../mbedutils/src/rpc/proto
    ${PROJECT_SOURCE_DIR}/../lib/cpputest/include
    ${PROJECT_SOURCE_DIR}/../lib/etl/include
    ${PROJECT_SOURCE_DIR}/expect
    ${PROJECT_SOURCE_DIR}/mock
)

create_test_target(
    TARGET
        UnitTestMbedutils
    TEST_SOURCES
        run_all_tests.cpp
        src/memory/nvm/test_nor_adesto.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/memory/nvm/nor_adesto.cpp
    DEPENDENT_SOURCES
        ${PROJECT_SOURCE_DIR}/mock/gpio_intf_mock.cpp
        ${PROJECT_SOURCE_DIR}/mock/spi_intf_mock.cpp
        ${PROJECT_SOURCE_DIR}/mock/time_intf_mock.cpp
        ${PROJECT_SOURCE_DIR}/expect/gpio_intf_expect.cpp
        ${PROJECT_SOURCE_DIR}/expect/spi_intf_expect.cpp
        ${PROJECT_SOURCE_DIR}/expect/time_intf_expect.cpp
    INCLUDE_DIRS
        ${INCLUDE_DIRECTORIES}
    LIBRARIES
        mbedutils_lib_stl
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
