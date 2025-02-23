project(mbedutils_test)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG)
endif()

# Import CppUTest
add_subdirectory(${PROJECT_SOURCE_DIR}/../lib/cpputest ${CMAKE_CURRENT_BINARY_DIR}/cpputest)

# Include Dependent Headers
set(TST_CMN_INC_DIRS
  ${PROJECT_SOURCE_DIR}/../
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

# Common Dependent Sources for Testing. These are meant to be included in all tests and
# should not contain any runtime specific code or code which could be tested in other
# tests.
set(TST_CMN_DEP_SOURCES
  ${PROJECT_SOURCE_DIR}/harness/test_runtime_harness.cpp
  ${PROJECT_SOURCE_DIR}/../mbedutils/src/core/nanoprintf.cpp
  ${PROJECT_SOURCE_DIR}/../mbedutils/src/system/system_intf.cpp
)

# Integration Test Sources for Simulator runtime
set(TST_INT_SOURCES_SIM
  ${MBEDUTILS_TEST_EXPECT_DIR}/assert_intf_expect.cpp
  ${MBEDUTILS_TEST_EXPECT_DIR}/irq_intf_expect.cpp
  ${MBEDUTILS_TEST_MOCK_DIR}/assert_intf_mock.cpp
  ${MBEDUTILS_TEST_MOCK_DIR}/irq_intf_mock.cpp
  ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/sim_mutex.cpp
  ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/sim_smphr.cpp
  ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/sim_thread.cpp
  ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/sim_time.cpp
  ${PROJECT_SOURCE_DIR}/../mbedutils/src/core/assert.cpp
  ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/condition.cpp
  ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/message.cpp
  ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/thread.cpp
  ${TST_CMN_DEP_SOURCES}
)

# Integration Test Sources for FreeRTOS runtime
set(TST_INT_SOURCES_FREERTOS
  ${MBEDUTILS_TEST_EXPECT_DIR}/assert_intf_expect.cpp
  ${MBEDUTILS_TEST_EXPECT_DIR}/irq_intf_expect.cpp
  ${MBEDUTILS_TEST_MOCK_DIR}/assert_intf_mock.cpp
  ${MBEDUTILS_TEST_MOCK_DIR}/irq_intf_mock.cpp
  ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_mutex.cpp
  ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_smphr.cpp
  ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_thread.cpp
  ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_time.cpp
  ${PROJECT_SOURCE_DIR}/../mbedutils/src/core/assert.cpp
  ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/condition.cpp
  ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/message.cpp
  ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/thread.cpp
  ${TST_CMN_DEP_SOURCES}
)

# Modify the mbedutils headers to target to include the etl library. It's not packaged
# with the mbedutils library, so we need to include it separately.
target_include_directories(mbedutils_headers
    INTERFACE
    ${PROJECT_SOURCE_DIR}/../lib/etl/include
)

add_subdirectory(harness/freertos)
add_subdirectory(src/database/test_key_value_db)
add_subdirectory(src/database/test_key_value_db_mt)
add_subdirectory(src/interfaces/test_intf_mutex)
add_subdirectory(src/interfaces/test_intf_smphr)
add_subdirectory(src/interfaces/test_intf_thread)
add_subdirectory(src/logging/test_tsdb_sink)
add_subdirectory(src/memory/nvm/test_nor_adesto)
add_subdirectory(src/memory/nvm/test_nor_flash)
add_subdirectory(src/threading/test_condition)
add_subdirectory(src/threading/test_message)
add_subdirectory(src/threading/test_thread)

add_custom_target(BuildAllTests)
add_dependencies(BuildAllTests
  IntTest_Interface_Mutex_FreeRTOS
  IntTest_Interface_Mutex_SIM
  IntTest_Interface_Smphr_FreeRTOS
  IntTest_Interface_Smphr_SIM
  IntTest_Interface_Thread_SIM
  IntTest_Thread_Condition_FreeRTOS
  IntTest_Thread_Condition_SIM
  IntTest_Thread_Thread_FreeRTOS
  IntTest_Thread_Thread_SIM
  UnitTest_Database_KeyValueDatabase
  UnitTest_Database_KeyValueDatabaseMT
  UnitTest_Logging_TSDBSink
  UnitTest_Memory_NVM_NorAdesto
  UnitTest_Memory_NVM_NorFlash
  UnitTest_Thread_Message
  # UnitTest_Thread_Thread
)
