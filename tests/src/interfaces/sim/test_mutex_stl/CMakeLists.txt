include(${MBEDUTILS_TEST_DIR}/test_target.cmake)
create_test_target(
    TARGET
        TestIntfMutexStl
    TEST_SOURCES
        test_mutex_stl.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/mutex_stl.cpp
    INCLUDE_DIRS
        ${TESTING_INCLUDE_DIRECTORIES}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
