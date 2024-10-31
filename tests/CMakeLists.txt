project(mbedutils_test)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG)
endif()

# Import CppUTest
add_subdirectory(${PROJECT_SOURCE_DIR}/../lib/cpputest ${CMAKE_CURRENT_BINARY_DIR}/cpputest)

# Include Dependent Headers
set(TESTING_COMMON_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/../mbedutils/include
  ${PROJECT_SOURCE_DIR}/../mbedutils/lib/cobs
  ${PROJECT_SOURCE_DIR}/../mbedutils/lib/nanopb
  ${PROJECT_SOURCE_DIR}/../mbedutils/lib/nanoprintf
  ${PROJECT_SOURCE_DIR}/../mbedutils/src/rpc/proto
  ${PROJECT_SOURCE_DIR}/../lib/cpputest/include
  ${PROJECT_SOURCE_DIR}/../lib/etl/include
  ${MBEDUTILS_TEST_EXPECT_DIR}
  ${MBEDUTILS_TEST_FAKE_DIR}
  ${MBEDUTILS_TEST_HARNESS_DIR}
  ${MBEDUTILS_TEST_MOCK_DIR}
)

# Common Sources for Testing
set(TESTING_COMMON_SOURCES
  ${PROJECT_SOURCE_DIR}/../mbedutils/src/core/nanoprintf.cpp
)

# Modify the mbedutils headers to target to include the etl library. It's not packaged
# with the mbedutils library, so we need to include it separately.
target_include_directories(mbedutils_headers
    INTERFACE
    ${PROJECT_SOURCE_DIR}/../lib/etl/include
)

add_subdirectory(harness/freertos)
add_subdirectory(src/threading/test_condition)
add_subdirectory(src/threading/test_message)
add_subdirectory(src/threading/test_thread)
add_subdirectory(src/database/test_key_value_db)
add_subdirectory(src/database/test_key_value_db_mt)
add_subdirectory(src/interfaces/sim/test_mutex_stl)
add_subdirectory(src/interfaces/sim/test_smphr_stl)
add_subdirectory(src/interfaces/sim/test_thread_stl)
add_subdirectory(src/memory/nvm/test_nor_adesto)
add_subdirectory(src/memory/nvm/test_nor_flash)

add_custom_target(BuildAllTests)
add_dependencies(BuildAllTests
  TestIntfMutexStl
  TestIntfSmphrStl
  TestIntfThreadStl
  TestKeyValueDatabase
  TestKeyValueDatabaseMT
  TestNorAdesto
  TestNorFlash
  TestThreadingCondition
  TestThreadingCondition_Integration_STL
  TestThreadingMessage
  TestThreadingThread
  TestThreadingThread_Integration_STL
)
