include(${MBEDUTILS_TEST_DIR}/test_target.cmake)
create_test_target(
    TARGET
        UnitTest_Database_KeyValueDatabase
    TEST_SOURCES
        test_key_value_db.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/database/db_kv_node.cpp
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/database/db_kv_nvm.cpp
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/database/db_kv_ram.cpp
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/database/db_kv_util.cpp
    DEPENDENT_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/test_kv_db.pb.c
        ${MBEDUTILS_TEST_EXPECT_DIR}/assert_intf_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/atexit_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/gpio_intf_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/logging_driver_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/mutex_intf_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/nor_flash_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/spi_intf_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/time_intf_expect.cpp
        ${MBEDUTILS_TEST_FAKE_DIR}/assert_fake.cpp
        ${MBEDUTILS_TEST_FAKE_DIR}/nor_flash_file.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/assert_intf_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/atexit_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/gpio_intf_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/logging_driver_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/mutex_intf_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/nor_flash_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/spi_intf_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/time_intf_mock.cpp
        ${PROJECT_SOURCE_DIR}/../mbedutils/lib/flashdb/port/fal/src/fal.c
        ${PROJECT_SOURCE_DIR}/../mbedutils/lib/flashdb/port/fal/src/fal_flash.c
        ${PROJECT_SOURCE_DIR}/../mbedutils/lib/flashdb/port/fal/src/fal_partition.c
        ${PROJECT_SOURCE_DIR}/../mbedutils/lib/flashdb/src/fdb.c
        ${PROJECT_SOURCE_DIR}/../mbedutils/lib/flashdb/src/fdb_kvdb.c
        ${PROJECT_SOURCE_DIR}/../mbedutils/lib/flashdb/src/fdb_tsdb.c
        ${PROJECT_SOURCE_DIR}/../mbedutils/lib/flashdb/src/fdb_utils.c
        ${PROJECT_SOURCE_DIR}/../mbedutils/lib/nanopb/pb_common.c
        ${PROJECT_SOURCE_DIR}/../mbedutils/lib/nanopb/pb_decode.c
        ${PROJECT_SOURCE_DIR}/../mbedutils/lib/nanopb/pb_encode.c
        ${TST_CMN_DEP_SOURCES}
    INCLUDE_DIRS
        ./
        ${TST_CMN_INC_DIRS}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
