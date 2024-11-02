include(${MBEDUTILS_TEST_DIR}/test_target.cmake)

# # Unit Tests
# create_test_target(
#     TARGET
#         TestThreadingCondition
#     TEST_SOURCES
#         test_condition.cpp
#     INSTRUMENTED_SOURCES
#         ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/condition.cpp
#     DEPENDENT_SOURCES
#         ${MBEDUTILS_TEST_EXPECT_DIR}/assert_expect.cpp
#         ${MBEDUTILS_TEST_EXPECT_DIR}/mutex_intf_expect.cpp
#         ${MBEDUTILS_TEST_EXPECT_DIR}/smphr_intf_expect.cpp
#         ${MBEDUTILS_TEST_EXPECT_DIR}/thread_intf_expect.cpp
#         ${MBEDUTILS_TEST_MOCK_DIR}/assert_mock.cpp
#         ${MBEDUTILS_TEST_MOCK_DIR}/mutex_intf_mock.cpp
#         ${MBEDUTILS_TEST_MOCK_DIR}/smphr_intf_mock.cpp
#         ${MBEDUTILS_TEST_MOCK_DIR}/thread_intf_mock.cpp
#         ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/thread.cpp
#         ${TST_CMN_DEP_SOURCES}
#     INCLUDE_DIRS
#         ${TST_CMN_INC_DIRS}
#     LIBRARIES
#         mbedutils_headers
#         mbedutils_internal_headers
#     EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
# )

# STL Integration Test
create_test_target(
    TARGET
        IntTest_Thread_Condition_STL
    TEST_SOURCES
        test_condition_integration.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/condition.cpp
    DEPENDENT_SOURCES
        ${TST_INT_SOURCES_SIM}
    INCLUDE_DIRS
        ${TST_CMN_INC_DIRS}
    DEFINES
        -DINTEGRATION_TEST
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

# FreeRTOS Integration Test
create_test_target(
    TARGET
        IntTest_Thread_Condition_FreeRTOS
    TEST_SOURCES
        test_condition_integration.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/condition.cpp
    DEPENDENT_SOURCES
        ${TST_INT_SOURCES_FREERTOS}
    INCLUDE_DIRS
        ${TST_CMN_INC_DIRS}
    DEFINES
        -DINTEGRATION_TEST
        -DMBEDUTILS_TEST_RUNTIME_FREERTOS
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
        mbedutils_test_lib_freertos
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)