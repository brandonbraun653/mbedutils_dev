include(test_target.cmake)

project(mbedutils_test)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG)
endif()

# Import CppUTest
add_subdirectory(${PROJECT_SOURCE_DIR}/../lib/cpputest ${CMAKE_CURRENT_BINARY_DIR}/cpputest)

# Include Dependent Headers
set(TESTING_INCLUDE_DIRECTORIES
    ${PROJECT_SOURCE_DIR}/../mbedutils/include
    ${PROJECT_SOURCE_DIR}/../mbedutils/lib/cobs
    ${PROJECT_SOURCE_DIR}/../mbedutils/lib/nanopb
    ${PROJECT_SOURCE_DIR}/../mbedutils/src/rpc/proto
    ${PROJECT_SOURCE_DIR}/../lib/cpputest/include
    ${PROJECT_SOURCE_DIR}/../lib/etl/include
    ${PROJECT_SOURCE_DIR}/expect
    ${PROJECT_SOURCE_DIR}/mock
)

add_subdirectory(src/memory/nvm/test_nor_adesto)
add_subdirectory(src/memory/nvm/test_nor_flash)

add_custom_target(BuildAllTests)
add_dependencies(BuildAllTests TestNorAdesto TestNorFlash)
