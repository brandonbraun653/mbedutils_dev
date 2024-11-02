include(${MBEDUTILS_TEST_DIR}/test_target.cmake)
# create_test_target(
#     TARGET
#         UnitTest_Thread_Thread
#     TEST_SOURCES
#         test_thread.cpp
#     INSTRUMENTED_SOURCES
#         ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/thread.cpp
#     DEPENDENT_SOURCES
#         ${MBEDUTILS_TEST_EXPECT_DIR}/assert_expect.cpp
#         ${MBEDUTILS_TEST_EXPECT_DIR}/mutex_intf_expect.cpp
#         ${MBEDUTILS_TEST_EXPECT_DIR}/smphr_intf_expect.cpp
#         ${MBEDUTILS_TEST_EXPECT_DIR}/thread_intf_expect.cpp
#         ${MBEDUTILS_TEST_EXPECT_DIR}/time_intf_expect.cpp
#         ${MBEDUTILS_TEST_MOCK_DIR}/assert_mock.cpp
#         ${MBEDUTILS_TEST_MOCK_DIR}/mutex_intf_mock.cpp
#         ${MBEDUTILS_TEST_MOCK_DIR}/smphr_intf_mock.cpp
#         ${MBEDUTILS_TEST_MOCK_DIR}/thread_intf_mock.cpp
#         ${MBEDUTILS_TEST_MOCK_DIR}/time_intf_mock.cpp
#         ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/condition.cpp
#         ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/message.cpp
#         ${TST_CMN_DEP_SOURCES}
#     INCLUDE_DIRS
#         ./
#         ${TST_CMN_INC_DIRS}
#     LIBRARIES
#         mbedutils_headers
#         mbedutils_internal_headers
#     EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
# )

create_test_target(
    TARGET
        IntTest_Thread_Thread_SIM
    TEST_SOURCES
        test_thread_integration.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/thread.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/sim_thread.cpp
    DEPENDENT_SOURCES
        ${MBEDUTILS_TEST_EXPECT_DIR}/assert_intf_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/irq_intf_expect.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/assert_intf_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/irq_intf_mock.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/sim_mutex.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/sim_smphr.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/sim_time.cpp
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/core/assert.cpp
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/condition.cpp
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/message.cpp
        ${TST_CMN_DEP_SOURCES}
    INCLUDE_DIRS
        ${TST_CMN_INC_DIRS}
    DEFINES
        -DINTEGRATION_TEST
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

create_test_target(
    TARGET
        IntTest_Thread_Thread_FreeRTOS
    TEST_SOURCES
        test_thread_integration.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/thread.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_thread.cpp
    DEPENDENT_SOURCES
        ${MBEDUTILS_TEST_EXPECT_DIR}/assert_intf_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/irq_intf_expect.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/assert_intf_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/irq_intf_mock.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_mutex.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_smphr.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_time.cpp
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/core/assert.cpp
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/condition.cpp
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/message.cpp
        ${TST_CMN_DEP_SOURCES}
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

