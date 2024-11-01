include(${MBEDUTILS_TEST_DIR}/test_target.cmake)
create_test_target(
    TARGET
        Test_Interface_Thread_STL
    TEST_SOURCES
        test_intf_thread.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/thread_stl.cpp
    INCLUDE_DIRS
        ${TESTING_COMMON_INCLUDE_DIRS}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
