include(${PROJECT_SOURCE_DIR}/test_target.cmake)
create_test_target(
    TARGET
        TestIntfThreadStl
    TEST_SOURCES
        test_thread_stl.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/thread_stl.cpp
    INCLUDE_DIRS
        ${TESTING_INCLUDE_DIRECTORIES}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
