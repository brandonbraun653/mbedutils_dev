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
    ${PROJECT_SOURCE_DIR}/fake
    ${PROJECT_SOURCE_DIR}/harness
    ${PROJECT_SOURCE_DIR}/mock
)

# Modify the mbedutils headers to target to include the etl library. It's not packaged
# with the mbedutils library, so we need to include it separately.
target_include_directories(mbedutils_headers
    INTERFACE
    ${PROJECT_SOURCE_DIR}/../lib/etl/include
)


add_subdirectory(src/interfaces/sim/test_mutex_stl)
add_subdirectory(src/interfaces/sim/test_smphr_stl)
add_subdirectory(src/database/test_key_value_db)
add_subdirectory(src/database/test_key_value_db_mt)
add_subdirectory(src/memory/nvm/test_nor_adesto)
add_subdirectory(src/memory/nvm/test_nor_flash)

add_custom_target(BuildAllTests)
add_dependencies(BuildAllTests
  TestIntfMutexStl
  TestIntfSmphrStl
  TestKeyValueDatabase
  TestKeyValueDatabaseMT
  TestNorAdesto
  TestNorFlash
)
