include(${MBEDUTILS_TEST_DIR}/test_target.cmake)

# Unit Tests
create_test_target(
    TARGET
        TestThreadingCondition
    TEST_SOURCES
        test_condition.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/condition.cpp
    DEPENDENT_SOURCES
        ${MBEDUTILS_TEST_EXPECT_DIR}/assert_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/mutex_intf_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/smphr_intf_expect.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/assert_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/mutex_intf_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/smphr_intf_mock.cpp
        ${TESTING_COMMON_SOURCES}
    INCLUDE_DIRS
        ${TESTING_COMMON_INCLUDE_DIRS}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

# STL Integration Test
create_test_target(
    TARGET
        TestThreadingCondition_Integration_STL
    TEST_SOURCES
        test_condition_integration.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/condition.cpp
    DEPENDENT_SOURCES
        ${MBEDUTILS_TEST_FAKE_DIR}/assert_fake.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/mutex_stl.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/smphr_stl.cpp
        ${TESTING_COMMON_SOURCES}
    INCLUDE_DIRS
        ${TESTING_COMMON_INCLUDE_DIRS}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

# FreeRTOS Integration Test
create_test_target(
    TARGET
        TestThreadingCondition_Integration_FreeRTOS
    TEST_SOURCES
        test_condition_integration.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/condition.cpp
    DEPENDENT_SOURCES
        ${MBEDUTILS_TEST_FAKE_DIR}/assert_fake.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_mutex.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_smphr.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_thread.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_time.cpp
        ${TESTING_COMMON_SOURCES}
    INCLUDE_DIRS
        ${TESTING_COMMON_INCLUDE_DIRS}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
        mbedutils_test_lib_freertos
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)