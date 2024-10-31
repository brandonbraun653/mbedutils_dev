include(${MBEDUTILS_TEST_DIR}/test_target.cmake)
create_test_target(
    TARGET
        Test_Interface_Mutex_STL
    TEST_SOURCES
        test_intf_mutex.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/mutex_stl.cpp
    INCLUDE_DIRS
        ${TESTING_COMMON_INCLUDE_DIRS}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

create_test_target(
    TARGET
        Test_Interface_Mutex_FreeRTOS
    TEST_SOURCES
        test_intf_mutex.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_mutex.cpp
    DEPENDENT_SOURCES
        ${MBEDUTILS_TEST_FAKE_DIR}/assert_fake.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_smphr.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_thread.cpp
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_freertos/freertos_time.cpp
    INCLUDE_DIRS
        ${TESTING_COMMON_INCLUDE_DIRS}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
        mbedutils_test_lib_freertos
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
