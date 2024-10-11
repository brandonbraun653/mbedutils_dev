include(${PROJECT_SOURCE_DIR}/test_target.cmake)
create_test_target(
    TARGET
        TestIntfSmphrStl
    TEST_SOURCES
        test_smphr_stl.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/smphr_stl.cpp
    INCLUDE_DIRS
        ${TESTING_INCLUDE_DIRECTORIES}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
