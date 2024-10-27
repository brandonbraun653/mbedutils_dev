include(${MBEDUTILS_TEST_DIR}/test_target.cmake)
create_test_target(
    TARGET
        TestThreadingThread
    TEST_SOURCES
        test_thread.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/thread.cpp
    DEPENDENT_SOURCES
        ${TESTING_COMMON_SOURCES}
        ${MBEDUTILS_TEST_EXPECT_DIR}/assert_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/mutex_intf_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/thread_intf_expect.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/assert_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/mutex_intf_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/thread_intf_mock.cpp
    INCLUDE_DIRS
        ./
        ${TESTING_COMMON_INCLUDE_DIRS}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

# Todo: Create functional test cases as well?
# ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/thread_stl.cpp

create_test_target(
    TARGET
        TestThreading_ThisThread_STL
    TEST_SOURCES
        test_this_thread.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/thread.cpp
    DEPENDENT_SOURCES
        ${TESTING_COMMON_SOURCES}
        ${MBEDUTILS_TEST_EXPECT_DIR}/assert_expect.cpp
        ${MBEDUTILS_TEST_EXPECT_DIR}/mutex_intf_expect.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/assert_mock.cpp
        ${MBEDUTILS_TEST_MOCK_DIR}/mutex_intf_mock.cpp
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/threading/message.cpp

        # Configure the test to use the STL implementation of Task()
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/thread_stl.cpp
    INCLUDE_DIRS
        ${TESTING_COMMON_INCLUDE_DIRS}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
