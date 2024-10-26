include(${MBEDUTILS_TEST_DIR}/test_target.cmake)
create_test_target(
    TARGET
        TestCoreThreadSTL
    TEST_SOURCES
        test_core_thread_stl.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../mbedutils/src/core/thread.cpp
    DEPENDENT_SOURCES
        ${TESTING_COMMON_SOURCES}
    INCLUDE_DIRS
        ${TESTING_COMMON_INCLUDE_DIRS}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
